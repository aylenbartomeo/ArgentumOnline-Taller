#ifndef COMBAT_MANAGER_H
#define COMBAT_MANAGER_H

#include <memory>
#include <unordered_map>

#include "AttackStrategy.h"
#include "MagicStrategy.h"
#include "MeleeStrategy.h"
#include "RangedStrategy.h"
#include "../entities/Player.h"
#include "../entities/Monster.h"

class CombatManager {
private:
    std::unordered_map<WeaponType, std::unique_ptr<AttackStrategy>> strategies;

    // Constructor privado para garantizar que solo exista una instancia (Singleton)
    CombatManager() {
        strategies[WeaponType::MELEE] = std::make_unique<MeleeStrategy>();
        strategies[WeaponType::RANGED] = std::make_unique<RangedStrategy>();
        strategies[WeaponType::MAGIC] = std::make_unique<MagicStrategy>();
    }
    const float PVP_CRITICAL_PROBABILITY = 0.05f;

public:
    CombatManager(const CombatManager&) = delete;
    CombatManager& operator=(const CombatManager&) = delete;

    static CombatManager& getInstance() {
        static CombatManager instance;
        return instance;
    }

    // 1. El jugador inicia la acción (PvP o PvE)
    void processAttack(Player& attacker, Attackable& target, Weapon* weapon);
};

#endif
