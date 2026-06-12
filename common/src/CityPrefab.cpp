#include "CityPrefab.h"

#include <utility>

#include "OverlayRegistry.h"
#include "TerrainRegistry.h"

CityPrefab makeCityPrefab() {
    CityPrefab city;
    city.width = 10;
    city.height = 10;

    for (int dy = 0; dy < 10; ++dy) {
        for (int dx = 0; dx < 10; ++dx) {
            city.terrain.push_back({dx, dy, TerrainCode::STONE});
        }
    }
    auto setTerrain = [&city](int dx, int dy, int code) {
        for (auto& c : city.terrain) {
            if (c.dx == dx && c.dy == dy) {
                c.code = code;
                return;
            }
        }
    };
    setTerrain(1, 7, TerrainCode::WOOD);
    setTerrain(8, 7, TerrainCode::WOOD);

    const std::vector<std::pair<int, int>> walls = {
            {3, 0}, {4, 0}, {5, 0}, {3, 1}, {5, 1}, {3, 2}, {5, 2},
            {0, 6}, {2, 6}, {0, 7}, {2, 7}, {0, 8}, {1, 8}, {2, 8},
            {7, 6}, {9, 6}, {7, 7}, {9, 7}, {7, 8}, {8, 8}, {9, 8},
    };
    for (const auto& w : walls) {
        city.overlays.push_back({w.first, w.second, OverlayTile::WALL});
    }
    city.overlays.push_back({4, 1, OverlayTile::ALTAR});
    city.overlays.push_back({1, 7, OverlayTile::BOVEDA});
    city.overlays.push_back({8, 7, OverlayTile::MOSTRADOR});

    city.npcs.push_back({"priest", 4, 1});
    city.npcs.push_back({"banker", 1, 7});
    city.npcs.push_back({"merchant", 8, 7});

    return city;
}
