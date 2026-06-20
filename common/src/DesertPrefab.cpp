#include "DesertPrefab.h"

namespace {
constexpr int SIZE = 16;
constexpr int ARENA = 74;
constexpr int CACTUS = 137;
constexpr int CARTEL = 162;

void addCactus(DesertPrefab& p, int dx, int dy) {
    p.decoration.push_back({dx, dy, CACTUS});
    p.obstacles.push_back({dx, dy, 0});
}

DesertPrefab buildPrefab() {
    DesertPrefab p;
    p.width = SIZE;
    p.height = SIZE;
    for (int dy = 0; dy < SIZE; ++dy) {
        for (int dx = 0; dx < SIZE; ++dx) {
            p.ground.push_back({dx, dy, ARENA});
        }
    }
    addCactus(p, 3, 3);
    addCactus(p, 12, 2);
    addCactus(p, 6, 6);
    addCactus(p, 13, 8);
    addCactus(p, 2, 10);
    addCactus(p, 9, 12);
    addCactus(p, 5, 13);
    addCactus(p, 14, 13);
    p.decoration.push_back({1, 14, CARTEL});
    return p;
}
}  // namespace

const DesertPrefab& getDesertPrefab() {
    static const DesertPrefab prefab = buildPrefab();
    return prefab;
}
