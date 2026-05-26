#include "Camera.h"

#include <algorithm>

Camera::Camera(int viewportWidth, int viewportHeight, int tileScreenSize, int mapCols,
               int mapRows):
        viewportWidth(viewportWidth),
        viewportHeight(viewportHeight),
        tileScreenSize(tileScreenSize),
        mapCols(mapCols),
        mapRows(mapRows),
        offsetX(0),
        offsetY(0) {}

void Camera::clamp() {
    int maxX = std::max(0, mapCols * tileScreenSize - viewportWidth);
    int maxY = std::max(0, mapRows * tileScreenSize - viewportHeight);
    offsetX = std::max(0, std::min(offsetX, maxX));
    offsetY = std::max(0, std::min(offsetY, maxY));
}

void Camera::move(int dxPx, int dyPx) {
    offsetX += dxPx;
    offsetY += dyPx;
    clamp();
}

void Camera::setMapSize(int newCols, int newRows) {
    mapCols = newCols;
    mapRows = newRows;
    clamp();
}

Position Camera::screenToCell(int screenX, int screenY) const {
    return {(screenX + offsetX) / tileScreenSize, (screenY + offsetY) / tileScreenSize};
}

Position Camera::cellToScreen(int col, int row) const {
    return {col * tileScreenSize - offsetX, row * tileScreenSize - offsetY};
}

int Camera::getOffsetX() const { return offsetX; }
int Camera::getOffsetY() const { return offsetY; }
