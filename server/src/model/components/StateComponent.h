#ifndef STATE_COMPONENT_H
#define STATE_COMPONENT_H

#include <memory>

#include "states/GhostState.h"
#include "states/MeditatingState.h"
#include "states/NormalState.h"
#include "states/PlayerState.h"

class StateComponent {
private:
    std::unique_ptr<PlayerState> currentState;

public:
    // Al nacer/loguear, el jugador arranca en estado normal
    StateComponent(): currentState(std::make_unique<NormalState>()) {}

    // --- TRANSICIONES DE ESTADO ---
    void die() {
        if (!currentState->isGhost()) {
            currentState = std::make_unique<GhostState>();
        }
    }

    void resurrect() {
        if (currentState->isGhost()) {
            currentState = std::make_unique<NormalState>();
        }
    }

    void startMeditating() {
        if (!currentState->isGhost() && !currentState->isMeditating()) {
            currentState = std::make_unique<MeditatingState>();
        }
    }

    void stopMeditating() {
        if (currentState->isMeditating()) {
            currentState = std::make_unique<NormalState>();
        }
    }

    // --- DELEGACIÓN DE COMPORTAMIENTO (Filtros) ---
    bool canMove() const { return currentState->canMove(); }
    bool canAttack() const { return currentState->canAttack(); }
    bool canUseItems() const { return currentState->canUseItems(); }

    bool isGhost() const { return currentState->isGhost(); }
    bool isMeditating() const { return currentState->isMeditating(); }

    bool canBeAttacked() const { return currentState->canBeAttacked(); }
};


#endif
