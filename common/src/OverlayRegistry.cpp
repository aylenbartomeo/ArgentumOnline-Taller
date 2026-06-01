#include "OverlayRegistry.h"

const std::vector<OverlayDef>& getOverlayRegistry() {
    static const std::vector<OverlayDef> registry = {
            {"5137.png", 604, 283, 39, 35}, {"5137.png", 707, 221, 25, 35},
            {"5137.png", 703, 358, 27, 25}, {"5137.png", 805, 230, 18, 25},
            {"5137.png", 607, 232, 34, 23},
    };
    return registry;
}
