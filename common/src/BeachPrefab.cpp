#include "BeachPrefab.h"

namespace {
constexpr int WIDTH = 22;
constexpr int HEIGHT = 18;
constexpr int WX0 = 2;
constexpr int WY0 = 2;
constexpr int WX1 = 19;
constexpr int WY1 = 15;
constexpr int FX0 = 2;
constexpr int FY0 = 2;
constexpr int FX1 = 18;
constexpr int FY1 = 14;
constexpr int WATER = 109;
constexpr int PALM = 42;
constexpr int FOAM_TOP = 88;
constexpr int FOAM_BOTTOM = 90;
constexpr int FOAM_LEFT = 89;
constexpr int FOAM_RIGHT = 91;
constexpr int FOAM_TL = 87;
constexpr int FOAM_TR = 92;
constexpr int FOAM_BL = 86;
constexpr int FOAM_BR = 93;

void addPalm(BeachPrefab& p, int dx, int dy) {
    p.decoration.push_back({dx, dy, PALM});
    p.obstacles.push_back({dx, dy, 0});
}

BeachPrefab buildPrefab() {
    BeachPrefab p;
    p.width = WIDTH;
    p.height = HEIGHT;
    for (int dy = WY0; dy <= WY1; ++dy) {
        for (int dx = WX0; dx <= WX1; ++dx) {
            p.ground.push_back({dx, dy, WATER});
        }
    }
    for (int dy = WY0 + 1; dy <= WY1 - 1; ++dy) {
        for (int dx = WX0 + 1; dx <= WX1 - 1; ++dx) {
            p.obstacles.push_back({dx, dy, 0});
        }
    }
    for (int dx = FX0 + 1; dx <= FX1 - 1; ++dx) {
        p.ground2.push_back({dx, FY0, FOAM_TOP});
        p.ground2.push_back({dx, FY1, FOAM_BOTTOM});
    }
    for (int dy = FY0 + 1; dy <= FY1 - 1; ++dy) {
        p.ground2.push_back({FX0, dy, FOAM_LEFT});
        p.ground2.push_back({FX1, dy, FOAM_RIGHT});
    }
    p.ground2.push_back({FX0, FY0, FOAM_TL});
    p.ground2.push_back({FX1, FY0, FOAM_TR});
    p.ground2.push_back({FX0, FY1, FOAM_BL});
    p.ground2.push_back({FX1, FY1, FOAM_BR});
    addPalm(p, 0, 1);
    addPalm(p, 9, 1);
    addPalm(p, 17, 1);
    addPalm(p, 0, 8);
    addPalm(p, 17, 8);
    addPalm(p, 0, 16);
    addPalm(p, 9, 16);
    addPalm(p, 17, 16);
    return p;
}
}  // namespace

const BeachPrefab& getBeachPrefab() {
    static const BeachPrefab prefab = buildPrefab();
    return prefab;
}
