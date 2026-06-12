#include "TerrainRegistry.h"

const std::vector<TerrainDef>& getTerrainRegistry() {
    static const std::vector<TerrainDef> registry = {
            {"ground/107.png", "pasto"},
            {"ground/108.png", "agua"},
            {"ground/16.png", "piedra"},
            {"ground/105.png", "madera"},
            {"ground/73.png", "arena"},
    };
    return registry;
}
