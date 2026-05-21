#ifndef COMBAT_COMPONENT_H
#define COMBAT_COMPONENT_H

#include "StatsComponent.h"
#include "StateComponent.h"

class CombatComponent {
private:
    StatsComponent& stats;
    StateComponent& state;

public:
    CombatComponent(StatsComponent& statsRef, StateComponent& stateRef);

    void takeDamage(uint16_t finalDamage);

    void heal(uint16_t amount);
};

#endif