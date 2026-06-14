#include "BuildingFronts.h"

std::optional<int> buildingFrontTiles(int decorationSpriteId) {
    switch (decorationSpriteId) {
        case 201:
            return 6;
        case 202:
            return 4;
        default:
            return std::nullopt;
    }
}

int roofDropPixels(int roofSpriteId) {
    switch (roofSpriteId) {
        case 204:
            return 200;
        case 205:
            return 194;
        default:
            return 0;
    }
}
