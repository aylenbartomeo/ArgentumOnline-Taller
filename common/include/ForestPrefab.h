#ifndef FOREST_PREFAB_H
#define FOREST_PREFAB_H

#include <vector>

struct ForestCell {
    int dx;
    int dy;
    int value;
};

struct ForestPrefab {
    int width;
    int height;
    std::vector<ForestCell> decoration;
    std::vector<ForestCell> obstacles;
};

const ForestPrefab& getForestPrefab();

#endif
