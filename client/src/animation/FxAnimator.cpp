#include "FxAnimator.h"

int fxFrameIndex(uint32_t elapsedMs, uint32_t frameDurMs, int frameCount) {
    if (frameDurMs == 0 || frameCount <= 0)
        return -1;
    const int index = static_cast<int>(elapsedMs / frameDurMs);
    if (index >= frameCount)
        return -1;
    return index;
}

FrameRect fxFrameRect(int index, int frameW, int frameH, int cols) {
    const int safeCols = cols > 0 ? cols : 1;
    const int col = index % safeCols;
    const int row = index / safeCols;
    return FrameRect{col * frameW, row * frameH, frameW, frameH};
}
