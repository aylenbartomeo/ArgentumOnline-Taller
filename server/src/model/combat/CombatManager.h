#ifndef COMBAT_MANAGER_H
#define COMBAT_MANAGER_H

#include <unordered_map>
#include <memory>
#include "AttackStrategy.h"
#include "MeleeStrategy.h"
#include "RangedStrategy.h"
#include "MagicStrategy.h"

class CombatManager {
private:
    std::unordered_map<WeaponType, std::unique_ptr<AttackStrategy>> strategies;

public:
    CombatManager() {
        strategies[WeaponType::MELEE] = std::make_unique<MeleeStrategy>();
        strategies[WeaponType::RANGED] = std::make_unique<RangedStrategy>();
        strategies[WeaponType::MAGIC] = std::make_unique<MagicStrategy>();
    }

    bool executeAttack(const Weapon& weapon, Combatant& attacker, Combatant& target, FormulaEngine& formulas) {
        auto it = strategies.find(weapon.getType());
        if (it != strategies.end()) {
            return it->second->execute(weapon, attacker, target, formulas);
        }
        return false;
    }
};

#endif