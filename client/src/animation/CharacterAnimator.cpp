#include "CharacterAnimator.h"

#include <cstdlib>

#include "Motion.h"

namespace {
constexpr int FRAME_COLS = 6;
constexpr int CELL_X[FRAME_COLS] = {2, 27, 52, 76, 101, 126};
constexpr int ROW_Y[4] = {4, 48, 92, 137};
constexpr int BODY_FRAME_W = 24;
constexpr int BODY_FRAME_H = 44;
constexpr int HEAD_X = 6;
constexpr int HEAD_W = 13;
constexpr int HEAD_H = 15;
constexpr int HEAD_ROW_Y[4] = {13, 77, 141, 205};

constexpr uint32_t MOVE_TIMEOUT_MS = 200;
constexpr uint32_t FRAME_MS = 120;
constexpr int WALK_FRAMES = 4;
constexpr float MOVE_SPEED = 0.005f;
constexpr float SNAP_DIST = 2.5f;
}  // namespace

int rowForFacing(Movement facing) {
    switch (facing) {
        case Movement::UP:
            return 1;
        case Movement::LEFT:
            return 2;
        case Movement::RIGHT:
            return 3;
        default:
            return 0;
    }
}

Movement directionFromDelta(int dx, int dy) {
    if (std::abs(dx) >= std::abs(dy)) {
        return dx >= 0 ? Movement::RIGHT : Movement::LEFT;
    }
    return dy >= 0 ? Movement::DOWN : Movement::UP;
}

FrameRect bodyFrameRect(Movement facing, int frameCol) {
    int col = frameCol;
    if (col < 0) {
        col = 0;
    }
    if (col >= FRAME_COLS) {
        col = FRAME_COLS - 1;
    }
    return FrameRect{CELL_X[col], ROW_Y[rowForFacing(facing)], BODY_FRAME_W, BODY_FRAME_H};
}

FrameRect headFrameRect(Movement facing) {
    return FrameRect{HEAD_X, HEAD_ROW_Y[rowForFacing(facing)], HEAD_W, HEAD_H};
}

void CharacterAnimator::update(int x, int y, uint32_t nowMs) {
    if (hasPrev && (x != lastX || y != lastY)) {
        facing = directionFromDelta(x - lastX, y - lastY);
        lastStepMs = nowMs;
    }
    const Vec2 target{static_cast<float>(x), static_cast<float>(y)};
    if (!hasVirt) {
        virtX = target.x;
        virtY = target.y;
        hasVirt = true;
    } else {
        const Vec2 next =
                stepToward({virtX, virtY}, target, MOVE_SPEED, nowMs - lastUpdateMs, SNAP_DIST);
        virtX = next.x;
        virtY = next.y;
    }
    lastUpdateMs = nowMs;
    lastX = x;
    lastY = y;
    hasPrev = true;
}

Movement CharacterAnimator::getFacing() const { return facing; }

int CharacterAnimator::frameColumn(uint32_t nowMs) const {
    if (hasPrev && nowMs - lastStepMs < MOVE_TIMEOUT_MS) {
        return 1 + static_cast<int>((nowMs / FRAME_MS) % WALK_FRAMES);
    }
    return 0;
}

float CharacterAnimator::getVirtualX() const { return virtX; }

float CharacterAnimator::getVirtualY() const { return virtY; }
