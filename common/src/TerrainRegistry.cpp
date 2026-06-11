#include "TerrainRegistry.h"

const std::vector<TerrainDef>& getTerrainRegistry() {
    static const std::vector<TerrainDef> registry = {
            {"5108.png", 416, 384, "pasto"},
            {"5108.png", 448, 128, "agua"},
            {"5108.png", 416, 384, "camino"},
            {"5108.png", 512, 480, "empedrado"},
            {"5108.png", 512, 480, "pisoMadera"},
            {"5108.png", 512, 480, "pisoPiedra"},
    };
    return registry;
}
