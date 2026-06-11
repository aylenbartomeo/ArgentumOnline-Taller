#include "CityStamp.h"

#include "CityPrefab.h"
#include "TerrainRegistry.h"

void applyCityPrefab(EditorMap& map, int originX, int originY, const std::string& name) {
    CityPrefab city = makeCityPrefab();
    for (const auto& t : city.terrain) {
        map.setTerrain(originX + t.dx, originY + t.dy, t.code);
    }
    for (const auto& o : city.overlays) {
        map.setTile(originX + o.dx, originY + o.dy, o.tile);
    }
    for (const auto& n : city.npcs) {
        map.addCitizen(n.type, originX + n.dx, originY + n.dy);
    }
    map.addSafeZone(name, originX, originY, city.width, city.height);
}

void clearCity(EditorMap& map, int originX, int originY) {
    CityPrefab city = makeCityPrefab();
    for (int dy = 0; dy < city.height; ++dy) {
        for (int dx = 0; dx < city.width; ++dx) {
            map.setTerrain(originX + dx, originY + dy, TerrainCode::GRASS);
            map.setTile(originX + dx, originY + dy, 0);
        }
    }
    for (const auto& n : city.npcs) {
        map.removeEntitiesAt(originX + n.dx, originY + n.dy);
    }
    map.removeSafeZoneAt(originX, originY);
}
