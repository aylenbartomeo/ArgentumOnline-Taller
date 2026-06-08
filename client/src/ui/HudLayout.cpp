#include "HudLayout.h"

int barFillWidth(int current, int max, int fullPx) {
    if (max <= 0 || current <= 0) {
        return 0;
    }
    if (current >= max) {
        return fullPx;
    }
    return current * fullPx / max;
}

SlotRect inventorySlotRect(int slotIndex, int cols, int cellPx, int gapPx, int originX,
                           int originY) {
    const int col = slotIndex % cols;
    const int row = slotIndex / cols;
    return SlotRect{originX + col * (cellPx + gapPx), originY + row * (cellPx + gapPx), cellPx,
                    cellPx};
}
