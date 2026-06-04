#include "Viewport.h"

#include <algorithm>

namespace {
int clampOffset(int focusPx, int view, int world) {
    const int maxOffset = std::max(0, world - view);
    return std::clamp(focusPx - view / 2, 0, maxOffset);
}
}  // namespace

CameraOffset computeCameraOffset(int focusPxX, int focusPxY, int viewW, int viewH, int worldW,
                                 int worldH) {
    return CameraOffset{clampOffset(focusPxX, viewW, worldW), clampOffset(focusPxY, viewH, worldH)};
}
