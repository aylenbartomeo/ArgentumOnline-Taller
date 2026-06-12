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
constexpr int STONE = 2;
constexpr int WOOD = 3;
constexpr int SAND = 4;
}  // namespace TerrainCode

#endif
