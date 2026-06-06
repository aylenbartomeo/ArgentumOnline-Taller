#include "Motion.h"

#include <cmath>

Vec2 stepToward(Vec2 current, Vec2 target, float speedTilesPerMs, uint32_t dtMs,
                float snapDistTiles) {
    const float dx = target.x - current.x;
    const float dy = target.y - current.y;
    const float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 1e-6f) {
        return current;
    }
    if (dist > snapDistTiles) {
        return target;
    }
    const float step = speedTilesPerMs * static_cast<float>(dtMs);
    if (step >= dist) {
        return target;
    }
    const float ratio = step / dist;
    return Vec2{current.x + dx * ratio, current.y + dy * ratio};
}
