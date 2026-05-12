#ifndef COMBAT_MANAGER_H
#define COMBAT_MANAGER_H

#include <memory>
#include <unordered_map>

#include "AttackStrategy.h"
#include "MagicStrategy.h"
#include "MeleeStrategy.h"
#include "RangedStrategy.h"

class CombatManager {
private:
    std::unordered_map<WeaponType, std::unique_ptr<AttackStrategy>> strategies;

public:
    CombatManager() {
        strategies[WeaponType::MELEE] = std::make_unique<MeleeStrategy>();
        strategies[WeaponType::RANGED] = std::make_unique<RangedStrategy>();
        strategies[WeaponType::MAGIC] = std::make_unique<MagicStrategy>();
    }

    bool executeAttack(const Weapon& weapon, Combatant& attacker, Combatant& target) {
        auto it = strategies.find(weapon.getType());
        if (it != strategies.end()) {
            return it->second->execute(weapon, attacker, target);
        }
        return false;
    }
};

#endif
