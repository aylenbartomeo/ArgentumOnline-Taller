#ifndef FX_ANIMATOR_H
#define FX_ANIMATOR_H

#include <cstdint>

#include "CharacterAnimator.h"

int fxFrameIndex(uint32_t elapsedMs, uint32_t frameDurMs, int frameCount);

FrameRect fxFrameRect(int index, int frameW, int frameH, int cols);

#endif
