#include "Camera.h"

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace {
const int ZOOM_LEVELS[] = {12, 16, 24, 32, 48};
const int ZOOM_COUNT = 5;

int nearestZoomIndex(int tileSize) {
    int best = 0;
    int bestDist = std::abs(ZOOM_LEVELS[0] - tileSize);
    for (int i = 1; i < ZOOM_COUNT; ++i) {
        int d = std::abs(ZOOM_LEVELS[i] - tileSize);
        if (d < bestDist) {
            bestDist = d;
            best = i;
        }
    }
    return best;
}
}  // namespace

Camera::Camera(int viewportWidth, int viewportHeight, int tileScreenSize, int mapCols, int mapRows):
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

void Camera::setTileSizePreservingCenter(int newTileSize) {
    double centerCol = (offsetX + viewportWidth / 2.0) / tileScreenSize;
    double centerRow = (offsetY + viewportHeight / 2.0) / tileScreenSize;
    tileScreenSize = newTileSize;
    offsetX = static_cast<int>(centerCol * tileScreenSize - viewportWidth / 2.0);
    offsetY = static_cast<int>(centerRow * tileScreenSize - viewportHeight / 2.0);
    clamp();
}

void Camera::zoomIn() {
    int idx = nearestZoomIndex(tileScreenSize);
    if (idx < ZOOM_COUNT - 1) {
        setTileSizePreservingCenter(ZOOM_LEVELS[idx + 1]);
    }
}

void Camera::zoomOut() {
    int idx = nearestZoomIndex(tileScreenSize);
    if (idx > 0) {
        setTileSizePreservingCenter(ZOOM_LEVELS[idx - 1]);
    }
}

int Camera::getTileSize() const { return tileScreenSize; }
