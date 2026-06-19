#include "ForestPrefab.h"

namespace {
constexpr int OAK = 11;
constexpr int SIZE = 12;

void addOak(ForestPrefab& p, int dx, int dy) {
    p.decoration.push_back({dx, dy, OAK});
    p.obstacles.push_back({dx + 3, dy, 0});
    p.obstacles.push_back({dx + 4, dy, 0});
}

ForestPrefab buildPrefab() {
    ForestPrefab p;
    p.width = SIZE;
    p.height = SIZE;
    addOak(p, 0, 5);
    addOak(p, 3, 8);
    addOak(p, 6, 11);
    addOak(p, 7, 7);
    addOak(p, 1, 11);
    addOak(p, 5, 5);
    addOak(p, 7, 10);
    return p;
}
}  // namespace

const ForestPrefab& getForestPrefab() {
    static const ForestPrefab prefab = buildPrefab();
    return prefab;
}
