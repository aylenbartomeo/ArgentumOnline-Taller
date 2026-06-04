#ifndef CHARACTER_ANIMATOR_H
#define CHARACTER_ANIMATOR_H

#include <cstdint>

#include "common/include/dto/CommandDTO.h"

struct FrameRect {
    int x;
    int y;
    int w;
    int h;
};

Movement directionFromDelta(int dx, int dy);

FrameRect bodyFrameRect(Movement facing, int frameCol);
FrameRect headFrameRect(Movement facing);

class CharacterAnimator {
public:
    void update(int x, int y, uint32_t nowMs);
    Movement getFacing() const;
    int frameColumn(uint32_t nowMs) const;

private:
    int lastX = 0;
    int lastY = 0;
    bool hasPrev = false;
    Movement facing = Movement::DOWN;
    uint32_t lastStepMs = 0;
};

#endif
