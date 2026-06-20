#ifndef DESERT_PREFAB_H
#define DESERT_PREFAB_H

#include <vector>

struct DesertCell {
    int dx;
    int dy;
    int value;
};

struct DesertPrefab {
    int width;
    int height;
    std::vector<DesertCell> ground;
    std::vector<DesertCell> decoration;
    std::vector<DesertCell> obstacles;
};

const DesertPrefab& getDesertPrefab();

#endif
