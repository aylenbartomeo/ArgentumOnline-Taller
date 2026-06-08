#ifndef MOTION_H
#define MOTION_H

#include <cstdint>

struct Vec2 {
    float x;
    float y;
};

Vec2 stepToward(Vec2 current, Vec2 target, float speedTilesPerMs, uint32_t dtMs,
                float snapDistTiles);

#endif
