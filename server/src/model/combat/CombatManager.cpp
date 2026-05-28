#include "CombatManager.h"

#include <algorithm>
#include <iostream>

#include "../entities/Monster.h"
#include "../entities/Player.h"
#include "../items/Weapon.h"
#include "model/FormulaEngine.h"

// --- Lógica compartida de combate ---

CombatResult CombatManager::resolveCombat(const Attackable& attacker, Attackable& target,
                                          const AttackParams& params) {
    CombatResult res;
    // 1. Validar distancia
    if (attacker.distance_to(target) > params.attackRange) {
        return res;  // attackHappened = false
    }

    // 2. Validar que el target pueda ser atacado
    if (target.isDead() || !target.canBeAttacked()) {
        return res;
    }

    res.attackHappened = true;

    // 3. Calcular daño bruto usando la Fuerza
    uint16_t rawDamage = FormulaEngine::getInstance().calculate_base_damage(
            attacker.getStrength(), params.minDamage, params.maxDamage);

    // 4. Chequear crítico
    const float CRITICAL_PROB = 0.05f;
    res.critical = FormulaEngine::getInstance().is_critical_attack(CRITICAL_PROB);
    if (res.critical) {
        rawDamage *= 2;
    }

    // 5. Chequear esquive (si no fue crítico, no se puede esquivar)
    if (!res.critical && FormulaEngine::getInstance().is_attack_eluded(target.getAgility())) {
        res.evaded = true;
        return res;
    }

    // 6. Calcular defensa y daño final
    int defense = target.getDefense();
    res.damage = std::max(0, static_cast<int>(rawDamage) - defense);

    // 7. Aplicar daño
    target.receiveDamage(res.damage);

    return res;
}

// --- Player ataca ---

CombatResult CombatManager::processAttack(Player& attacker, Attackable& target) {
    // Obtener el arma equipada del jugador
    const Weapon* weapon = attacker.getEquippedWeapon();
    if (!weapon) {
        return CombatResult{};
    }

    // Armar los parámetros de ataque a partir del arma
    AttackParams params{static_cast<uint16_t>(weapon->getMinDamage()),
                        static_cast<uint16_t>(weapon->getMaxDamage()), weapon->getAttackRange(),
                        weapon->getManaCost(), weapon->getType() == WeaponType::MAGIC};

    // Chequeo de maná si el arma es mágica
    if (params.isMagic) {
        if (!attacker.consumeMana(params.manaCost)) {
            return CombatResult{};
        }
    }

    // Resolver combate (lógica compartida)
    CombatResult res = resolveCombat(attacker, target, params);
    if (!res.attackHappened || res.evaded)
        return res;

    // Experiencia por ataque (solo Players ganan XP)
    uint32_t attackXp = FormulaEngine::getInstance().calculate_attack_xp_gain(
            res.damage, attacker.getLevel(), target.getLevel());
    attacker.addExperience(attackXp);

    // Si el target muere, XP extra + lógica de muerte
    if (target.isDead()) {
        target.handleDeath();

        uint32_t killXp = FormulaEngine::getInstance().calculate_kill_xp_gain(
                target.getMaxHp(), attacker.getLevel(), target.getLevel());
        attacker.addExperience(killXp);
    }

    return res;
}

// --- Monster ataca ---

CombatResult CombatManager::processAttack(const Monster& attacker, Attackable& target) {
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

// --- Player ataca con bonus de clan ---
 
CombatResult CombatManager::processAttack(Player& attacker, Attackable& target,
                                           float attackBonus, float defenseBonus) {
    const Weapon* weapon = attacker.getEquippedWeapon();
    if (!weapon) return CombatResult{};
 
    AttackParams params{static_cast<uint16_t>(weapon->getMinDamage()), static_cast<uint16_t>(weapon->getMaxDamage()),
                        weapon->getAttackRange(), weapon->getManaCost(), weapon->getType() == WeaponType::MAGIC,
                        attackBonus, defenseBonus};
 
    if (params.isMagic) {
        if (!attacker.consumeMana(params.manaCost)) return CombatResult{};
    }
 
    CombatResult res = resolveCombat(attacker, target, params);
    if (!res.attackHappened || res.evaded) return res;
 
    uint32_t attackXp = FormulaEngine::getInstance().calculate_attack_xp_gain(
            res.damage, attacker.getLevel(), target.getLevel());
    attacker.addExperience(attackXp);
 
    if (target.isDead()) {
        target.handleDeath();
        uint32_t killXp = FormulaEngine::getInstance().calculate_kill_xp_gain(
                target.getMaxHp(), attacker.getLevel(), target.getLevel());
        attacker.addExperience(killXp);
    }
 
    return res;
}
