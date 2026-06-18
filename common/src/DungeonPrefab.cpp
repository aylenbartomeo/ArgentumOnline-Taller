#include "DungeonPrefab.h"

namespace {
constexpr int FLOOR = 4;
constexpr int LAVA = 45;
constexpr int GRAVE_HOLE = 102;
constexpr int GRAVE_SMALL = 100;
constexpr int SKELETON = 99;
constexpr int GOLD_ITEM_ID = 1;
constexpr int GOLD_AMOUNT = 5000;
constexpr int ARENA = 16;
constexpr int CORRIDOR_LEN = 6;

void floorCell(DungeonPrefab& p, int dx, int dy) { p.ground.push_back({dx, dy, FLOOR}); }

void lavaCell(DungeonPrefab& p, int dx, int dy) {
    p.ground.push_back({dx, dy, LAVA});
    p.obstacles.push_back({dx, dy, 0});
}

void addGrave(DungeonPrefab& p, int id, int dx, int dy) {
    p.decoration.push_back({dx, dy, id});
    int cells = (id == GRAVE_HOLE) ? 4 : 1;
    for (int k = 0; k < cells; ++k) {
        p.obstacles.push_back({dx, dy - k, 0});
    }
}

DungeonPrefab buildPrefab() {
    DungeonPrefab p;
    p.width = ARENA;
    p.height = ARENA + CORRIDOR_LEN;
    p.dungeonDx = 1;
    p.dungeonDy = 1;
    p.dungeonW = ARENA - 2;
    p.dungeonH = ARENA - 2;

    for (int dy = 0; dy < ARENA; ++dy) {
        for (int dx = 0; dx < ARENA; ++dx) {
            bool opening = (dy == ARENA - 1 && (dx == 7 || dx == 8));
            bool ring = (dy == 0 || dy == ARENA - 1 || dx == 0 || dx == ARENA - 1);
            if (ring && !opening) {
                lavaCell(p, dx, dy);
            } else {
                floorCell(p, dx, dy);
            }
        }
    }

    for (int dy = ARENA; dy < ARENA + CORRIDOR_LEN; ++dy) {
        floorCell(p, 7, dy);
        floorCell(p, 8, dy);
        lavaCell(p, 6, dy);
        lavaCell(p, 9, dy);
    }

    addGrave(p, GRAVE_HOLE, 2, 7);
    addGrave(p, GRAVE_HOLE, 13, 7);
    addGrave(p, GRAVE_SMALL, 2, 12);
    addGrave(p, GRAVE_SMALL, 13, 12);

    p.decoration.push_back({5, 4, SKELETON});
    p.decoration.push_back({10, 4, SKELETON});
    p.decoration.push_back({5, 11, SKELETON});
    p.decoration.push_back({10, 11, SKELETON});

    p.gold.push_back({7, 1, GOLD_ITEM_ID, GOLD_AMOUNT});
    p.gold.push_back({8, 1, GOLD_ITEM_ID, GOLD_AMOUNT});

    return p;
}
}  // namespace

const DungeonPrefab& getDungeonPrefab() {
    static const DungeonPrefab prefab = buildPrefab();
    return prefab;
}
