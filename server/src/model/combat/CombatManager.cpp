#include "CombatManager.h"

#include <algorithm>
#include <iostream>

#include "../entities/Monster.h"
#include "../entities/Player.h"
#include "../items/Weapon.h"
#include "model/FormulaEngine.h"

// --- Lógica compartida de combate ---

int CombatManager::resolveCombat(const Attackable& attacker, Attackable& target,
                                 const AttackParams& params) {
    // 1. Validar distancia
    if (attacker.distance_to(target) > params.attackRange) {
        std::cout << "[COMBAT] Target out of range." << std::endl;
        return -1;
    }

    // 2. Validar que el target pueda ser atacado
    if (target.isDead() || !target.canBeAttacked()) {
        std::cout << "[COMBAT] Target cannot be attacked." << std::endl;
        return -1;
    }

    // 3. Calcular daño bruto usando la Fuerza
    uint16_t rawDamage = FormulaEngine::getInstance().calculate_base_damage(
            attacker.getStrength(), params.minDamage, params.maxDamage);

    // 4. Chequear crítico
    const float CRITICAL_PROB = 0.05f;
    bool isCritical = FormulaEngine::getInstance().is_critical_attack(CRITICAL_PROB);
    if (isCritical) {
        rawDamage *= 2;
        std::cout << "[COMBAT] Critical hit! Damage: " << rawDamage << std::endl;
    }

    // 5. Chequear esquive (si no fue crítico, no se puede esquivar)
    if (!isCritical && FormulaEngine::getInstance().is_attack_eluded(target.getAgility())) {
        std::cout << "[COMBAT] Target evaded the attack!" << std::endl;
        return -1;
    }

    // 6. Calcular defensa y daño final
    int defense = target.getDefense();
    int finalDamage = std::max(0, static_cast<int>(rawDamage) - defense);

    std::cout << "[COMBAT] Dealt " << finalDamage << " damage (Raw: " << rawDamage
              << ", Def: " << defense << ")." << std::endl;

    // 7. Aplicar daño
    target.receiveDamage(finalDamage);

    return finalDamage;
}

// --- Player ataca ---

void CombatManager::processAttack(Player& attacker, Attackable& target) {
    // Obtener el arma equipada del jugador
    const Weapon* weapon = attacker.getEquippedWeapon();
    if (!weapon) {
        std::cout << "[COMBAT] Player has no weapon equipped." << std::endl;
        return;
    }

    // Armar los parámetros de ataque a partir del arma
    AttackParams params{static_cast<uint16_t>(weapon->getMinDamage()),
                        static_cast<uint16_t>(weapon->getMaxDamage()), weapon->getAttackRange(),
                        weapon->getManaCost(), weapon->getType() == WeaponType::MAGIC};

    // Chequeo de maná si el arma es mágica
    if (params.isMagic) {
        if (!attacker.consumeMana(params.manaCost)) {
            std::cout << "[COMBAT] Not enough mana." << std::endl;
            return;
        }
    }

    // Resolver combate (lógica compartida)
    int finalDamage = resolveCombat(attacker, target, params);
    if (finalDamage < 0)
        return;

    // Experiencia por ataque (solo Players ganan XP)
    uint32_t attackXp = FormulaEngine::getInstance().calculate_attack_xp_gain(
            finalDamage, attacker.getLevel(), target.getLevel());
    attacker.addExperience(attackXp);

    // Si el target muere, XP extra + lógica de muerte
    if (target.isDead()) {
        std::cout << "[COMBAT] Target died!" << std::endl;
        target.handleDeath();

        uint32_t killXp = FormulaEngine::getInstance().calculate_kill_xp_gain(
                target.getMaxHp(), attacker.getLevel(), target.getLevel());
        attacker.addExperience(killXp);
    }
}

// --- Monster ataca ---

void CombatManager::processAttack(const Monster& attacker, Attackable& target) {
    // Armar los parámetros de ataque a partir de los stats del monstruo
    AttackParams params{static_cast<uint16_t>(attacker.getAttackMin()),
                        static_cast<uint16_t>(attacker.getAttackMax()), attacker.get_attack_range(),
                        0,       // sin costo de maná
                        false};  // siempre físico

    // Resolver combate (lógica compartida)
    int finalDamage = resolveCombat(attacker, target, params);
    if (finalDamage < 0)
        return;

    // Si el target muere, ejecutar lógica de muerte (sin XP para monstruos)
    if (target.isDead()) {
        std::cout << "[COMBAT] Target died!" << std::endl;
        target.handleDeath();
    }
}
