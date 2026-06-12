#ifndef CITY_PREFAB_H
#define CITY_PREFAB_H

#include <string>
#include <vector>

struct CityTerrainCell {
    int dx;
    int dy;
    int code;
};

struct CityOverlayCell {
    int dx;
    int dy;
    int tile;
};

struct CityNpc {
    std::string type;
    int dx;
    int dy;
};

struct CityPrefab {
    int width;
    int height;
    std::vector<CityTerrainCell> terrain;
    std::vector<CityOverlayCell> overlays;
    std::vector<CityNpc> npcs;
};

CityPrefab makeCityPrefab();

#endif
