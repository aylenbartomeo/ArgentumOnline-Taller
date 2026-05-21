#ifndef GHOST_STATE_H
#define GHOST_STATE_H

#include "PlayerState.h"

class GhostState : public PlayerState {
public:
    GhostState() = default;
    ~GhostState() override = default;

    bool canMove() const override { return true; }
    bool canAttack() const override { return false; }
    bool canUseItems() const override { return false; }

    bool canBeAttacked() const override { return false; }

    bool isGhost() const override { return true; }
};

#endif
