#ifndef TERRAIN_REGISTRY_H
#define TERRAIN_REGISTRY_H

#include <string>
#include <vector>

struct TerrainDef {
    std::string sheet;
    std::string name;
};

const std::vector<TerrainDef>& getTerrainRegistry();

namespace TerrainCode {
constexpr int GRASS = 0;
constexpr int WATER = 1;
constexpr int ROAD = 2;
constexpr int COBBLE = 3;
constexpr int WOOD_FLOOR = 4;
constexpr int STONE_FLOOR = 5;
}  // namespace TerrainCode

#endif
