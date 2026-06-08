#ifndef HUD_LAYOUT_H
#define HUD_LAYOUT_H

struct SlotRect {
    int x;
    int y;
    int w;
    int h;
};

int barFillWidth(int current, int max, int fullPx);
SlotRect inventorySlotRect(int slotIndex, int cols, int cellPx, int gapPx, int originX,
                           int originY);

#endif
