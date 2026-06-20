#include "BeachPrefab.h"

namespace {
constexpr int WIDTH = 22;
constexpr int HEIGHT = 18;
constexpr int AX0 = 2;
constexpr int AY0 = 2;
constexpr int AX1 = 19;
constexpr int AY1 = 15;
constexpr int WATER = 109;
constexpr int PALM = 42;
constexpr int FOAM_TOP = 89;
constexpr int FOAM_BOTTOM = 87;
constexpr int FOAM_LEFT = 90;
constexpr int FOAM_RIGHT = 88;
constexpr int FOAM_TL = 96;
constexpr int FOAM_TR = 94;
constexpr int FOAM_BL = 95;
constexpr int FOAM_BR = 93;

void addPalm(BeachPrefab& p, int dx, int dy) {
    p.decoration.push_back({dx, dy, PALM});
    p.obstacles.push_back({dx, dy, 0});
}

BeachPrefab buildPrefab() {
    BeachPrefab p;
    p.width = WIDTH;
    p.height = HEIGHT;
    for (int dy = AY0; dy <= AY1; ++dy) {
        for (int dx = AX0; dx <= AX1; ++dx) {
            p.ground.push_back({dx, dy, WATER});
            p.obstacles.push_back({dx, dy, 0});
        }
    }
    for (int dx = AX0; dx <= AX1; ++dx) {
        p.ground2.push_back({dx, AY0 - 1, FOAM_TOP});
        p.ground2.push_back({dx, AY1 + 1, FOAM_BOTTOM});
    }
    for (int dy = AY0; dy <= AY1; ++dy) {
        p.ground2.push_back({AX0 - 1, dy, FOAM_LEFT});
        p.ground2.push_back({AX1 + 1, dy, FOAM_RIGHT});
    }
    p.ground2.push_back({AX0 - 1, AY0 - 1, FOAM_TL});
    p.ground2.push_back({AX1 + 1, AY0 - 1, FOAM_TR});
    p.ground2.push_back({AX0 - 1, AY1 + 1, FOAM_BL});
    p.ground2.push_back({AX1 + 1, AY1 + 1, FOAM_BR});
    addPalm(p, 0, 0);
    addPalm(p, 10, 0);
    addPalm(p, 21, 0);
    addPalm(p, 0, 8);
    addPalm(p, 21, 8);
    addPalm(p, 0, 17);
    addPalm(p, 10, 17);
    addPalm(p, 21, 17);
    return p;
}
}  // namespace

const BeachPrefab& getBeachPrefab() {
    static const BeachPrefab prefab = buildPrefab();
    return prefab;
}
