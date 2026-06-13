#ifndef OVERLAY_REGISTRY_H
#define OVERLAY_REGISTRY_H

#include <string>
#include <vector>

struct OverlayDef {
    std::string tilesheet;
    int srcX;
    int srcY;
    int srcW;
    int srcH;
    std::string name;
    int itemId;
    bool solid;
    bool stackable = false;
};

const std::vector<OverlayDef>& getOverlayRegistry();

namespace OverlayTile {
constexpr int WALL = 27;
constexpr int ALTAR = 28;
constexpr int MOSTRADOR = 29;
constexpr int BOVEDA = 30;
}  // namespace OverlayTile

#endif
