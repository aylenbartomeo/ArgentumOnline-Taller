#ifndef BEACH_PREFAB_H
#define BEACH_PREFAB_H

#include <vector>

struct BeachCell {
    int dx;
    int dy;
    int value;
};

struct BeachPrefab {
    int width;
    int height;
    std::vector<BeachCell> ground;
    std::vector<BeachCell> ground2;
    std::vector<BeachCell> decoration;
    std::vector<BeachCell> obstacles;
};

const BeachPrefab& getBeachPrefab();

#endif
