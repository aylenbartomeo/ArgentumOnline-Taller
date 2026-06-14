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

struct CityZone {
    std::string name;
    int dx;
    int dy;
    int width;
    int height;
};

struct CityPrefab {
    int width;
    int height;
    int safeDx = 0;
    int safeDy = 0;
    int safeW = 0;
    int safeH = 0;
    std::vector<CityCell> ground;
    std::vector<CityCell> decoration;
    std::vector<CityCell> roofs;
    std::vector<CityCell> indoor;
    std::vector<CityCell> obstacles;
    std::vector<CityNpc> npcs;
    std::vector<CityZone> buildings;
};

const CityPrefab& getCityPrefab();

#endif
