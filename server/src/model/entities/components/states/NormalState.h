#ifndef NORMAL_STATE_H
#define NORMAL_STATE_H

#include "PlayerState.h"

class NormalState : public PlayerState {
public:
    NormalState() = default;
    ~NormalState() override = default;

    bool canMove() const override { return true; }
    bool canAttack() const override { return true; }
    bool canUseItems() const override { return true; }

    bool canBeAttacked() const override { return true; }
};

#endif
