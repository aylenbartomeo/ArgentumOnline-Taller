#ifndef CITY_PREFAB_H
#define CITY_PREFAB_H

#include <string>
#include <vector>

struct CityCell {
    int dx;
    int dy;
    int value;
};

struct CityNpc {
    std::string type;
    int dx;
    int dy;
};

struct CityPrefab {
    int width;
    int height;
    std::vector<CityCell> ground;
    std::vector<CityCell> decoration;
    std::vector<CityCell> roofs;
    std::vector<CityCell> indoor;
    std::vector<CityCell> obstacles;
    std::vector<CityNpc> npcs;
};

const CityPrefab& getCityPrefab();

#endif
