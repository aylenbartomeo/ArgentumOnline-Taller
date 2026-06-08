#include "CombatSystem.h"

#include <string>

#include "../entities/Monster.h"
#include "../entities/Player.h"
#include "../items/Weapon.h"

#include "FormulaEngine.h"
#include "../interfaces/CombatStrategies.h"

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
    CombatResult res = processAttack(attacker, *target, attackBonus,
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

// --- Lógica compartida de combate ---

// Se encarga de validar si el ataque físico/inmediato es viable.
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

// Lógica de impacto compartida para ataques de Player y Monster. Aplica daño, calcula crítico/esquive, etc.
CombatResult CombatSystem::applyDamageEffect(Attackable& attacker, Attackable& target,
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

// --- Monster ataca ---

CombatResult CombatSystem::processAttack(const Monster& attacker, Attackable& target) {
    // Armar los parámetros de ataque a partir de los stats del monstruo
    AttackParams params{static_cast<uint16_t>(attacker.getAttackMin()),
                        static_cast<uint16_t>(attacker.getAttackMax()), attacker.get_attack_range(),
                        0,       // sin costo de maná
                        false};  // siempre físico

    // Resolver combate (lógica compartida)
    CombatResult res = resolveCombat(attacker, target, params);
    if (!res.attackHappened || res.evaded)
        return res;

    // Si el target muere, ejecutar lógica de muerte (sin XP para monstruos)
    if (target.isDead()) {
        target.handleDeath();
    }

    return res;
}

// --- Player ataca ---

CombatResult CombatSystem::processAttack(Player& attacker, Attackable& target, float attackBonus,
                                         float defenseBonus) {
    const Weapon* weapon = attacker.getEquippedWeapon();
    if (!weapon)
        return CombatResult{};

    // 1. Preparar los modificadores contextuales del combate (reemplaza parte de AttackParams)
    CombatModifiers modifiers{attackBonus, defenseBonus};

    // 2. DELEGACIÓN POLIMÓRFICA:
    // Dejamos que la estrategia de entrega maneje el flujo. 
    // Si la distancia es correcta y hay recursos, ejecutará el impacto e informará 'true'.
    bool attackHappened = weapon->getDelivery()->deliver(attacker, target, modifiers, *weapon, *this);

    // Creamos un resultado local básico para mantener la lógica de experiencia y muerte existente
    CombatResult res;
    res.attackHappened = attackHappened;

    if (!res.attackHappened)
        return res;

    // NOTA: Como la aplicación real del daño ahora ocurre dentro de apply() de forma diferida o inmediata,
    // para mantener el flujo de XP actual de la fase síncrona, asumimos que si el ataque ocurrió,
    // podemos procesar el estado de muerte del objetivo.
    
    // Deberiamos refactorizar esto dado que el impacto real se maneja en onProjectileHit, 
    // pero lo dejamos para no romper la lógica de XP actual.
    if (target.isDead()) {
        target.handleDeath();
        uint32_t killXp = FormulaEngine::getInstance().calculate_kill_xp_gain(
                target.getMaxHp(), attacker.getLevel(), target.getLevel());
        attacker.addExperience(killXp);
    }

    return res;
}

void CombatSystem::onProjectileHit(Attackable& attacker, Attackable& target, 
                                   IHitEffect* hitEffect, const CombatModifiers& modifiers, 
                                   const Weapon& weapon) {
    if (target.isDead() || !target.canBeAttacked()) {
        return; // Si el objetivo murió en el viaje del proyectil, se descarta el impacto
    }

    // Ejecución polimórfica diferida.
    // Si era un arco -> MeleeDamageEffect (daño físico)
    // Si era un bastón mágico -> MagicDamageEffect (valida/consume maná e impacta daño mágico)
    // FALTA: Si era un bastón de vida -> MagicHealEffect (cura al objetivo)
    if (hitEffect) {
        hitEffect->apply(attacker, target, modifiers, weapon, *this);
    }

    // --- MANEJO DE EXPERIENCIA DIFERIDA Y MUERTE ---
    // Movemos la lógica de recompensa que antes estaba en processAttack acá,
    // porque el impacto real ocurre en este frame, ticks después del disparo.
    Player* playerAttacker = dynamic_cast<Player*>(&attacker);
    if (playerAttacker) {
        // Nota: para el cálculo exacto de XP de ataque, podrías requerir el daño infligido.
        // Si tu applyDamageEffect devuelve el daño real aplicado, lo usás acá.
        
        if (target.isDead()) {
            target.handleDeath();
            uint32_t killXp = FormulaEngine::getInstance().calculate_kill_xp_gain(
                    target.getMaxHp(), playerAttacker->getLevel(), target.getLevel());
            playerAttacker->addExperience(killXp);
        }
    }
}
