#ifndef ATTACK_STRATEGY_H
#define ATTACK_STRATEGY_H

#include "../items/Weapon.h"
#include <iostream>

class Character; // (Reemplazar despues)

class AttackStrategy {
public:
    virtual ~AttackStrategy() = default;

    virtual bool execute(const Weapon& weapon /*, Character& attacker, Character& target */) = 0;
};

#endif