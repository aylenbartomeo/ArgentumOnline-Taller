#ifndef CAMERA_H
#define CAMERA_H

#include <utility>

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
    void setTileSizePreservingCenter(int newTileSize);

public:
    Camera(int viewportWidth, int viewportHeight, int tileScreenSize, int mapCols, int mapRows);

    void move(int dxPx, int dyPx);
    void setMapSize(int newCols, int newRows);

    void zoomIn();
    void zoomOut();
    int getTileSize() const;

    Position screenToCell(int screenX, int screenY) const;
    Position cellToScreen(int col, int row) const;

    int getOffsetX() const;
    int getOffsetY() const;
};

#endif
