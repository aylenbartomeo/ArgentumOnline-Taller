#include "TerrainRegistry.h"

const std::vector<TerrainDef>& getTerrainRegistry() {
    static const std::vector<TerrainDef> registry = {
            {"ground/107.png", "pasto"},
            {"ground/111.png", "agua"},
            {"ground/105.png", "camino"},
            {"ground/16.png", "empedrado"},
            {"ground/73.png", "pisoMadera"},
            {"ground/84.png", "pisoPiedra"},
    };
    return registry;
}
