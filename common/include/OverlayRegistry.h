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
};

const std::vector<OverlayDef>& getOverlayRegistry();

#endif
