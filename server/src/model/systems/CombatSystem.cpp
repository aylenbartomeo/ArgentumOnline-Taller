#include "CombatSystem.h"

#include <string>

#include "model/combat/CombatManager.h"

CombatSystem::CombatSystem(Map& map, EntityManager& em, ClanRepository& cr, EventPublisher& ep,
                           ICombatEventCallback& cb, bool enforceFairPlay):
        map(map),
        entityManager(em),
        clanRepo(cr),
        eventPublisher(ep),
        callback(cb),
        enforceFairPlay(enforceFairPlay) {}

bool CombatSystem::areClanmates(uint32_t dbId1, uint32_t dbId2) const {
    if (dbId1 == dbId2)
        return true;
    auto clan1 = clanRepo.getClanIdOfPlayer(dbId1);
    auto clan2 = clanRepo.getClanIdOfPlayer(dbId2);
    return clan1 && clan2 && *clan1 == *clan2;
}

int CombatSystem::countNearbyClanmates(uint32_t dbId, int range) const {
    auto clanId = clanRepo.getClanIdOfPlayer(dbId);
    if (!clanId)
        return 0;

    const Clan* clan = clanRepo.getClanById(*clanId);
    if (!clan)
        return 0;

    auto posOpt = entityManager.getPlayerPosition(dbId);
    if (!posOpt)
        return 0;

    int count = 0;
    for (uint32_t memberId: clan->getMembers()) {
        if (memberId == dbId)
            continue;
        auto memberPosOpt = entityManager.getPlayerPosition(memberId);
        if (memberPosOpt && posOpt->chebyshev_distance_to(*memberPosOpt) <= range) {
            count++;
        }
    }
    return count;
}

void CombatSystem::playerAttack(uint32_t attackerDbId, uint32_t targetDbId) {
    Player* pAttacker = entityManager.getPlayer(attackerDbId);
    if (!pAttacker)
        return;

    Player& attacker = *pAttacker;

    uint32_t targetEntityId = targetDbId;
    uint32_t resolvedTarget = entityManager.resolveEntityId(targetDbId);
    if (resolvedTarget != 0) {
        targetEntityId = resolvedTarget;
    }

    Attackable* target = entityManager.findAttackable(targetEntityId);
    if (!target)
        return;

    // --- Validar zona segura ---
    if (map.isSafeZone(attacker.getPosition().x, attacker.getPosition().y) ||
        map.isSafeZone(target->getPosition().x, target->getPosition().y)) {
        eventPublisher.sendTo(attackerDbId, "No puedes pelear en una zona segura.");
        return;
    }

    if (areClanmates(attackerDbId, targetDbId)) {
        eventPublisher.sendTo(attackerDbId, "No puedes atacar a un miembro de tu clan.");
        return;
    }

    // --- Validar que el atacante pueda atacar ---
    if (!attacker.canAttack()) {
        eventPublisher.sendTo(attackerDbId, "No puedes atacar en este momento.");
        return;
    }

    // --- Validar linea de vision ---
    if (!map.hasLineOfSight(attacker.getPosition(), target->getPosition())) {
        eventPublisher.sendTo(attackerDbId, "Hay un obstaculo bloqueando tu vision.");
        return;
    }

    if (enforceFairPlay &&
        (!attacker.canEngageInCombatWith(*target) || !target->canEngageInCombatWith(attacker))) {
        eventPublisher.sendTo(attackerDbId,
                              "No puedes pelear con este objetivo (violacion de fair play).");
        return;
    }

    // Variables de bonificación
    float attackBonus = 1.0f + (countNearbyClanmates(attackerDbId, CLAN_BONUS_RANGE) *
                                CLAN_ATTACK_BONUS_PER_MEMBER);
    float defenseBonus = 1.0f;

    // Notificar a los clanmates del target que está siendo atacado y aplicar su defensa
    auto targetPlayerIt = dynamic_cast<Player*>(target);
    if (targetPlayerIt) {
        uint32_t targetDb = targetPlayerIt->getDbId();

        defenseBonus +=
                countNearbyClanmates(targetDb, CLAN_BONUS_RANGE) * CLAN_DEFENSE_BONUS_PER_MEMBER;

        auto clanIdOpt = clanRepo.getClanIdOfPlayer(targetDb);
        if (clanIdOpt) {
            const Clan* clan = clanRepo.getClanById(*clanIdOpt);
            if (clan) {
                std::string alertMsg = "[Clan] " + targetPlayerIt->getName() +
                                       " está siendo atacado por " + attacker.getName() + "!";
                for (uint32_t memberId: clan->getMembers()) {
                    if (memberId != targetDb && memberId != attackerDbId) {
                        eventPublisher.sendTo(memberId, alertMsg);
                    }
                }
            }
        }
    }

    // Ejecutar ataque con bonificaciones calculadas
    CombatResult res = CombatManager::getInstance().processAttack(attacker, *target, attackBonus,
                                                                  defenseBonus);

    if (!res.attackHappened)
        return;

    if (res.evaded) {
        eventPublisher.sendTo(attackerDbId, "¡" + target->getName() + " evadió tu ataque!");
        if (targetPlayerIt) {
            eventPublisher.sendTo(targetPlayerIt->getDbId(),
                                  "¡Evadiste el ataque de " + attacker.getName() + "!");
        }
    } else {
        std::string critMsg = res.critical ? " ¡GOLPE CRITICO!" : "";
        eventPublisher.sendTo(attackerDbId, "¡Le hiciste " + std::to_string(res.damage) +
                                                    " de dano a " + target->getName() + "!" +
                                                    critMsg);
        if (targetPlayerIt) {
            eventPublisher.sendTo(targetPlayerIt->getDbId(),
                                  "¡Recibiste " + std::to_string(res.damage) + " de dano de " +
                                          attacker.getName() + "!");

            if (targetPlayerIt->isDead()) {
                std::string deathMsg =
                        attacker.getName() + " ha asesinado a " + targetPlayerIt->getName() + "!";
                eventPublisher.broadcast(deathMsg);
                callback.onPlayerDeath(targetPlayerIt->getDbId());
            }
        }

        const Monster* mTarget = dynamic_cast<const Monster*>(target);
        if (mTarget && mTarget->isDead()) {
            callback.onMonsterDeath(*mTarget, attackerDbId);
        }
    }
}

void CombatSystem::monsterAttack(const Monster& monster, Player& target) {
    // Validar zona segura
    if (map.isSafeZone(monster.getPosition().x, monster.getPosition().y) ||
        map.isSafeZone(target.getPosition().x, target.getPosition().y)) {
        return;
    }

    // Validar linea de visión
    if (!map.hasLineOfSight(monster.getPosition(), target.getPosition())) {
        return;
    }

    // Notificar a los clanmates del target (Player) que está siendo atacado
    uint32_t targetDb = target.getDbId();
    auto clanIdOpt = clanRepo.getClanIdOfPlayer(targetDb);
    if (clanIdOpt) {
        const Clan* clan = clanRepo.getClanById(*clanIdOpt);
        if (clan) {
            std::string alertMsg = "[Clan] " + target.getName() + " está siendo atacado por " +
                                   monster.getName() + "!";
            for (uint32_t memberId: clan->getMembers()) {
                if (memberId != targetDb) {
                    eventPublisher.sendTo(memberId, alertMsg);
                }
            }
        }
    }

    CombatResult res = CombatManager::getInstance().processAttack(monster, target);

    if (!res.attackHappened)
        return;

    if (res.evaded) {
        eventPublisher.sendTo(target.getDbId(),
                              "¡Evadiste el ataque de " + monster.getName() + "!");
    } else {
        eventPublisher.sendTo(target.getDbId(), "¡Recibiste " + std::to_string(res.damage) +
                                                        " de dano de " + monster.getName() + "!");
        if (target.isDead()) {
            std::string deathMsg = monster.getName() + " ha asesinado a " + target.getName() + "!";
            eventPublisher.broadcast(deathMsg);
            callback.onPlayerDeath(target.getDbId());
        }
    }
}
