#include "Death.h"

namespace {
constexpr int SKULL_TILE = 32;
constexpr int SKULL_COL = 4;
constexpr int SKULL_ROW = 1;
}  // namespace

bool isDead(int currentHp) { return currentHp <= 0; }

FrameRect skullFrameRect() {
    return FrameRect{(SKULL_COL - 1) * SKULL_TILE, (SKULL_ROW - 1) * SKULL_TILE, SKULL_TILE,
                     SKULL_TILE};
}
