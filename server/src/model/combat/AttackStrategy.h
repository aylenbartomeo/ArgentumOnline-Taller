#ifndef ATTACK_STRATEGY_H
#define ATTACK_STRATEGY_H

#include <iostream>

#include "../entities/Player.h"
#include "../interfaces/Attackable.h"
#include "../items/Weapon.h"
#include "model/FormulaEngine.h"

class AttackStrategy {
public:
    virtual ~AttackStrategy() = default;
    virtual bool execute(const Weapon& weapon, Player& attacker, Attackable& target) = 0;
};

#endif
