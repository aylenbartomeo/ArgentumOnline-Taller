#ifndef DEATH_H
#define DEATH_H

#include "CharacterAnimator.h"

bool isDead(int currentHp);
bool isGhost(uint8_t stateId);

FrameRect skullFrameRect();

#endif
