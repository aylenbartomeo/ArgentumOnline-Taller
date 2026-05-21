#ifndef ATTACK_STRATEGY_H
#define ATTACK_STRATEGY_H

#include <iostream>

#include "../entities/interfaces/Combatant.h"
#include "../items/Weapon.h"
#include "model/FormulaEngine.h"


class AttackStrategy {
public:
    virtual ~AttackStrategy() = default;
    virtual uint16_t calculateDamage(Combatant& attacker, Weapon* weapon) = 0;
    virtual bool execute(const Weapon& weapon, Combatant& attacker, Combatant& target) = 0;
};

#endif
