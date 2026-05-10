#ifndef ATTACK_STRATEGY_H
#define ATTACK_STRATEGY_H

#include "../items/Weapon.h"
#include <iostream>
#include "../entities/interfaces/combatant.h"
#include "model/FormulaEngine.h"


class AttackStrategy {
public:
    virtual ~AttackStrategy() = default;

    virtual bool execute(const Weapon& weapon, Combatant& attacker, Combatant& target, 
        FormulaEngine& formulas) = 0;
};

#endif
