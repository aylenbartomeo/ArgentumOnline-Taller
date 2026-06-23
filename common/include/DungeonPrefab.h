#ifndef DUNGEON_PREFAB_H
#define DUNGEON_PREFAB_H

#include <vector>

struct DungeonCell {
    int dx;
    int dy;
    int value;
};

struct DungeonItem {
    int dx;
    int dy;
    int itemId;
    int amount;
};

struct DungeonPrefab {
    int width;
    int height;
    int dungeonDx;
    int dungeonDy;
    int dungeonW;
    int dungeonH;
    std::vector<DungeonCell> ground;
    std::vector<DungeonCell> decoration;
    std::vector<DungeonCell> obstacles;
    std::vector<DungeonItem> gold;
};

const DungeonPrefab& getDungeonPrefab();

#endif
