#include "CombatComponent.h"
#include "StatsComponent.h"
#include "StateComponent.h"

CombatComponent::CombatComponent(StatsComponent& statsRef, StateComponent& stateRef)
    : stats(statsRef), state(stateRef) {}

void CombatComponent::takeDamage(uint16_t finalDamage) {
    if (stats.getHp() == 0) return; 

    stats.takeDamage(finalDamage);

    if (stats.getHp() == 0) {
        state.die(); 
    }
}

void CombatComponent::heal(uint16_t amount) {
    stats.heal(amount);
}