#include "CombatSystem.h"

#include <string>

#include "../entities/Monster.h"
#include "../entities/Player.h"
#include "../interfaces/CombatStrategies.h"
#include "../items/Weapon.h"

#include "BossSpawnSystem.h"
#include "FormulaEngine.h"

CombatSystem::CombatSystem(Map& map, EntityManager& em, ClanRepository& cr, EventPublisher& ep,
                           ICombatEventCallback& cb, const BossSpawnSystem& bss,
                           bool enforceFairPlay, const ServerConfig& config):
        map(map),
        entityManager(em),
        clanRepo(cr),
        eventPublisher(ep),
        callback(cb),
        bossSpawnSystem(bss),
        enforceFairPlay(enforceFairPlay),
        criticalProbability(config.criticalProbability),
        clanBonusCalc(em, cr, ep, config),
        notifier(ep, cb) {}

bool CombatSystem::checkFairPlay(const Player& attacker, const Attackable& target,
                                 uint32_t attackerDbId, bool notify) {
    if (enforceFairPlay &&
        (!attacker.canEngageInCombatWith(target) || !target.canEngageInCombatWith(attacker))) {
        if (notify) {
            eventPublisher.sendTo(attackerDbId,
                                  "No puedes pelear con este objetivo (violacion de fair play).");
        }
        return false;  // Violación de Fair Play
    }
    return true;  // Todo legal, proceda
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
    if (clanBonusCalc.areClanmates(attackerDbId, targetDbId)) {
        eventPublisher.sendTo(attackerDbId, "No puedes atacar a un miembro de tu clan.");
        return;
    }

    // --- Validar que el atacante pueda atacar ---
    if (!attacker.canAttack()) {
        eventPublisher.sendTo(attackerDbId, "No puedes atacar en este momento.");
        return;
    }

    // --- Validar boss area ---
    Monster* mTarget = dynamic_cast<Monster*>(target);
    if (mTarget && mTarget->isBoss()) {
        if (!bossSpawnSystem.isInAnyBossArea(attacker.getPosition())) {
            eventPublisher.sendTo(attackerDbId, "Debes acercarte al area del boss para atacarlo.");
            return;
        }
    }

    // --- Validar linea de vision ---
    if (!map.hasLineOfSight(attacker.getPosition(), target->getPosition())) {
        eventPublisher.sendTo(attackerDbId, "Hay un obstaculo bloqueando tu vision.");
        return;
    }

    // --- Validar fair play ---
    if (!checkFairPlay(attacker, *target, attackerDbId, true)) {
        return;
    }

    auto attackerPos = attacker.getPosition();
    auto targetPos = target->getPosition();
    const Weapon* weapon = attacker.getEquippedWeapon();

    if (!weapon || weapon->getType() == WeaponType::MELEE) {
        // Lógica Melee (Por celdas discretas / tiles de la grilla)
        int attackerTileX = static_cast<int>(std::floor(attackerPos.x));
        int attackerTileY = static_cast<int>(std::floor(attackerPos.y));
        int targetTileX = static_cast<int>(std::floor(targetPos.x));
        int targetTileY = static_cast<int>(std::floor(targetPos.y));

        int deltaX = std::abs(attackerTileX - targetTileX);
        int deltaY = std::abs(attackerTileY - targetTileY);

        // Permite atacar si está en cualquiera de los 8 casilleros adyacentes
        if (deltaX > 1 || deltaY > 1) {
            eventPublisher.sendTo(attackerDbId,
                                  "Estas demasiado lejos para atacar cuerpo a cuerpo.");
            return;
        }
    } else {
        float dx = attackerPos.x - targetPos.x;
        float dy = attackerPos.y - targetPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        int maxRange = weapon->getAttackRange();
        if (distance > maxRange) {
            eventPublisher.sendTo(attackerDbId, "El objetivo esta fuera del rango de tu arma.");
            return;
        }
    }

    // --- Calcular bonificaciones y notificar ataque de clan ---
    CombatModifiers mods = clanBonusCalc.buildModifiers(attackerDbId, target);

    // Ejecutar ataque con bonificaciones calculadas
    CombatResult res = processAttack(attacker, *target, mods.attackBonus, mods.defenseBonus);
    notifier.notifyCombatResult(attacker, *target, res);
    if (!res.attackHappened)
        return;

    attacker.setAction(static_cast<uint8_t>(EntityAction::ATTACKING), 400.0f);
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
    clanBonusCalc.notifyClanOfAttack(target.getDbId(), monster.getName());

    CombatResult res = processAttack(monster, target);

    if (!res.attackHappened)
        return;

    notifier.notifyCombatResult(monster, target, res);
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
    uint16_t rawDamage = FormulaEngine::getInstance().calculateBaseDamage(
            attacker.getStrength(), params.minDamage, params.maxDamage);

    rawDamage = static_cast<uint16_t>(rawDamage * params.attackBonus);

    // 2. Chequear crítico
    res.critical = FormulaEngine::getInstance().isCriticalAttack(criticalProbability);
    if (res.critical) {
        rawDamage *= 2;
    }

    // 3. Chequear esquive (si no fue crítico, no se puede esquivar)
    if (!res.critical && FormulaEngine::getInstance().isAttackEluded(target.getAgility())) {
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

CombatResult CombatSystem::applyHealEffect(Player& target) {
    CombatResult res;
    res.attackHappened = true;
    res.isHeal = true;
    target.restoreHp();
    return res;
}

CombatResult CombatSystem::processAttack(const Monster& attacker, Attackable& target) {
    AttackParams params{static_cast<uint16_t>(attacker.getAttackMin()),
                        static_cast<uint16_t>(attacker.getAttackMax()), attacker.getAttackRange(),
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

    CombatResult res = weapon->deliver(attacker, target, modifiers, *this);

    if (!res.attackHappened || res.isPending)
        return res;

    if (target.isDead()) {
        target.handleDeath();
        uint32_t killXp = FormulaEngine::getInstance().calculateKillXpGain(
                target.getMaxHp(), attacker.getLevel(), target.getLevel());
        attacker.addExperience(killXp);
    }

    return res;
}

// --- Impacto de proyectil (llamado por ProjectileSystem cuando el proyectil llega) ---
void CombatSystem::onProjectileHit(Player& attacker, Attackable& target, IHitEffect* hitEffect,
                                   const CombatModifiers& modifiers, const Weapon& weapon) {
    if (target.isDead() || !target.canBeAttacked()) {
        return;  // Si el objetivo murió en el viaje del proyectil, se descarta el impacto
    }

    // --- Validar boss area (el jugador podría haberse movido después de disparar) ---
    const Monster* mTarget = dynamic_cast<const Monster*>(&target);
    if (mTarget && mTarget->isBoss()) {
        if (!bossSpawnSystem.isInAnyBossArea(attacker.getPosition())) {
            eventPublisher.sendTo(attacker.getId(),
                                  "Debes acercarte al area del boss para atacarlo.");
            return;
        }
    }

    CombatResult res;
    if (hitEffect) {
        res = hitEffect->apply(attacker, target, modifiers, weapon, *this);
    }

    if (target.isDead()) {
        target.handleDeath();
        uint32_t killXp = FormulaEngine::getInstance().calculateKillXpGain(
                target.getMaxHp(), attacker.getLevel(), target.getLevel());
        attacker.addExperience(killXp);
    }

    notifier.notifyCombatResult(attacker, target, res);
}

CombatModifiers CombatSystem::buildModifiers(uint32_t attackerDbId,
                                             const Attackable* target) const {
    return clanBonusCalc.buildModifiers(attackerDbId, target);
}

bool CombatSystem::areClanmates(uint32_t dbId1, uint32_t dbId2) const {
    return clanBonusCalc.areClanmates(dbId1, dbId2);
}
