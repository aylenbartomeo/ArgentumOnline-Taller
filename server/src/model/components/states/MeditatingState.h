#ifndef MEDITATING_STATE_H
#define MEDITATING_STATE_H

#include "PlayerState.h"

class MeditatingState: public PlayerState {
public:
    MeditatingState() = default;
    ~MeditatingState() override = default;

    bool canMove() const override { return false; }
    bool canAttack() const override { return false; }
    bool canUseItems() const override { return false; }

    bool canBeAttacked() const override { return true; }

    bool isMeditating() const override { return true; }
};

#endif
