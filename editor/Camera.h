#ifndef CAMERA_H
#define CAMERA_H

#include "common/utils/position.h"

class Camera {
private:
    int viewportWidth;
    int viewportHeight;
    int tileScreenSize;
    int mapCols;
    int mapRows;
    int offsetX;
    int offsetY;

    void clamp();

public:
    Camera(int viewportWidth, int viewportHeight, int tileScreenSize, int mapCols, int mapRows);

    void move(int dxPx, int dyPx);
    void setMapSize(int newCols, int newRows);

    Position screenToCell(int screenX, int screenY) const;
    Position cellToScreen(int col, int row) const;

    int getOffsetX() const;
    int getOffsetY() const;
};

#endif
