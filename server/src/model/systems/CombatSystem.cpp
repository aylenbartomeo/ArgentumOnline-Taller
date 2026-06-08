#include "CombatSystem.h"

#include <string>

#include "../entities/Monster.h"
#include "../entities/Player.h"
#include "../interfaces/CombatStrategies.h"
#include "../items/Weapon.h"

#include "FormulaEngine.h"

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

    // -- Validar no atacar a un miembro de tu clan ---
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

    // --- Validar fair play ---
    if (enforceFairPlay &&
        (!attacker.canEngageInCombatWith(*target) || !target->canEngageInCombatWith(attacker))) {
        eventPublisher.sendTo(attackerDbId,
                              "No puedes pelear con este objetivo (violacion de fair play).");
        return;
    }

    // --- Calcular bonificaciones ---
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
    CombatResult res = processAttack(attacker, *target, attackBonus, defenseBonus);

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

    CombatResult res = processAttack(monster, target);

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


CombatResult CombatSystem::resolveCombat(const Attackable& attacker, Attackable& target,
                                         const AttackParams& params) {
    CombatResult res;

    // 1. Validar distancia (Esto es lo que el proyectil NO usará al impactar)
    if (attacker.distance_to(target) > params.attackRange) {
        return res;  // attackHappened = false
    }

    // 2. Validar que el target pueda ser atacado
    if (target.isDead() || !target.canBeAttacked()) {
        return res;
    }

    // Si pasó los filtros iniciales de inmediatez, aplica el efecto de daño
    return applyDamageEffect(const_cast<Attackable&>(attacker), target, params);
}

CombatResult CombatSystem::applyDamageEffect(const Attackable& attacker, Attackable& target,
                                             const AttackParams& params) {
    CombatResult res;
    res.attackHappened = true;

    // 1. Calcular daño bruto usando la Fuerza y aplicar bonificación de ataque del clan
    uint16_t rawDamage = FormulaEngine::getInstance().calculate_base_damage(
            attacker.getStrength(), params.minDamage, params.maxDamage);

    rawDamage = static_cast<uint16_t>(rawDamage * params.attackBonus);

    // 2. Chequear crítico
    const float CRITICAL_PROB = 0.05f;
    res.critical = FormulaEngine::getInstance().is_critical_attack(CRITICAL_PROB);
    if (res.critical) {
        rawDamage *= 2;
    }

    // 3. Chequear esquive (si no fue crítico, no se puede esquivar)
    if (!res.critical && FormulaEngine::getInstance().is_attack_eluded(target.getAgility())) {
        res.evaded = true;
        return res;
    }

    // 4. Calcular defensa, aplicar bonificación de defensa del clan y daño final
    int defense = target.getDefense();
    defense = static_cast<int>(defense * params.defenseBonus);

    res.damage = std::max(0, static_cast<int>(rawDamage) - defense);

    // 5. Aplicar daño físicamente en la entidad objetivo
    target.receiveDamage(res.damage);

    return res;
}


CombatResult CombatSystem::processAttack(const Monster& attacker, Attackable& target) {
    AttackParams params{static_cast<uint16_t>(attacker.getAttackMin()),
                        static_cast<uint16_t>(attacker.getAttackMax()), attacker.get_attack_range(),
                        0, false};

    CombatResult res = resolveCombat(attacker, target, params);
    if (!res.attackHappened || res.evaded)
        return res;

    if (target.isDead()) {
        target.handleDeath();
    }

    return res;
}

// --- Player ataca con bonuses de clan ---
CombatResult CombatSystem::processAttack(Player& attacker, Attackable& target, float attackBonus,
                                         float defenseBonus) {
    const Weapon* weapon = attacker.getEquippedWeapon();
    if (!weapon)
        return CombatResult{};

    CombatModifiers modifiers{attackBonus, defenseBonus};

    bool attackHappened =
            weapon->getDelivery()->deliver(attacker, target, modifiers, *weapon, *this);

    CombatResult res;
    res.attackHappened = attackHappened;

    if (!res.attackHappened)
        return res;

    if (target.isDead()) {
        target.handleDeath();
        uint32_t killXp = FormulaEngine::getInstance().calculate_kill_xp_gain(
                target.getMaxHp(), attacker.getLevel(), target.getLevel());
        attacker.addExperience(killXp);
    }

    return res;
}

// --- Impacto de proyectil (llamado por ProjectileSystem cuando el proyectil llega) ---
void CombatSystem::onProjectileHit(Attackable& attacker, Attackable& target, IHitEffect* hitEffect,
                                   const CombatModifiers& modifiers, const Weapon& weapon) {
    if (target.isDead() || !target.canBeAttacked()) {
        return;  // Si el objetivo murió en el viaje del proyectil, se descarta el impacto
    }

    // Ejecución polimórfica diferida.
    // Si era un arco         -> MeleeDamageEffect  (daño físico)
    // Si era un bastón mágico -> MagicDamageEffect (valida/consume maná e impacta daño mágico)
    if (hitEffect) {
        hitEffect->apply(attacker, target, modifiers, weapon, *this);
    }

    Player* playerAttacker = dynamic_cast<Player*>(&attacker);
    if (playerAttacker) {
        if (target.isDead()) {
            target.handleDeath();
            uint32_t killXp = FormulaEngine::getInstance().calculate_kill_xp_gain(
                    target.getMaxHp(), playerAttacker->getLevel(), target.getLevel());
            playerAttacker->addExperience(killXp);
        }
    }

    const Monster* mTarget = dynamic_cast<const Monster*>(&target);
    if (mTarget && mTarget->isDead()) {
        const Player* pAttacker = dynamic_cast<Player*>(&attacker);
        if (pAttacker)
            callback.onMonsterDeath(*mTarget, pAttacker->getDbId());
    }

    const Player* targetPlayer = dynamic_cast<Player*>(&target);
    if (targetPlayer && targetPlayer->isDead()) {
        callback.onPlayerDeath(targetPlayer->getDbId());
    }
}

CombatModifiers CombatSystem::buildModifiers(uint32_t attackerDbId,
                                             const Attackable* target) const {
    CombatModifiers m;
    m.attackBonus = 1.0f + (countNearbyClanmates(attackerDbId, CLAN_BONUS_RANGE) *
                            CLAN_ATTACK_BONUS_PER_MEMBER);
    m.defenseBonus = 1.0f;
    const Player* tp = dynamic_cast<const Player*>(target);
    if (tp)
        m.defenseBonus += countNearbyClanmates(tp->getDbId(), CLAN_BONUS_RANGE) *
                          CLAN_DEFENSE_BONUS_PER_MEMBER;
    return m;
}
