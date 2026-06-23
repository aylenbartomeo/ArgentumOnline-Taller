#include "BuildingFronts.h"

std::optional<int> buildingFrontTiles(int decorationSpriteId) {
    switch (decorationSpriteId) {
        case 201:
            return 6;
        case 202:
            return 5;
        default:
            return std::nullopt;
    }
}

int roofDropPixels(int roofSpriteId) {
    switch (roofSpriteId) {
        case 204:
            return 40;
        case 205:
            return 100;
        default:
            return 0;
    }
}

int roofShiftX(int roofSpriteId) {
    switch (roofSpriteId) {
        case 205:
            return -72;
        default:
            return 0;
    }
}
