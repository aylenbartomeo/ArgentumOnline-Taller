#include "CityStamp.h"

#include <algorithm>
#include <vector>

#include "CityPrefab.h"

namespace {
constexpr int WATER = 109;
constexpr int GRASS = 108;

std::string zoneNameForCitizen(const std::string& citizenType) {
    if (citizenType == "priest") {
        return "church";
    }
    if (citizenType == "banker") {
        return "bank";
    }
    if (citizenType == "merchant") {
        return "store";
    }
    return "";
}

std::string buildingErrorFor(const std::string& citizenType) {
    if (citizenType == "priest") {
        return "el priest va dentro de la iglesia";
    }
    if (citizenType == "banker") {
        return "el banker va dentro del banco";
    }
    if (citizenType == "merchant") {
        return "el merchant va alrededor de la tienda";
    }
    return "ese citizen no tiene lugar asignado";
}

const EditorSafeZone* zoneContaining(const EditorMap& map, int col, int row) {
    const auto& zones = map.getSafeZones();
    auto it = std::find_if(zones.begin(), zones.end(), [col, row](const EditorSafeZone& z) {
        return col >= z.x && col < z.x + z.width && row >= z.y && row < z.y + z.height;
    });
    return (it != zones.end()) ? &(*it) : nullptr;
}
}  // namespace

std::string cityStampError(const EditorMap& map, int originX, int originY) {
    const CityPrefab& prefab = getCityPrefab();
    if (originX < 0 || originY < 0 || originX + prefab.width > map.getWidth() ||
        originY + prefab.height > map.getHeight()) {
        return "la ciudad no entra en el mapa";
    }
    const std::vector<std::vector<int>>& ground = map.getGround();
    for (int y = originY; y < originY + prefab.height; ++y) {
        for (int x = originX; x < originX + prefab.width; ++x) {
            if (ground[y][x] == WATER) {
                return "la ciudad no puede pisar el agua";
            }
        }
    }
    for (const EditorSafeZone& zone: map.getSafeZones()) {
        bool apart = originX + prefab.width <= zone.x || zone.x + zone.width <= originX ||
                     originY + prefab.height <= zone.y || zone.y + zone.height <= originY;
        if (!apart) {
            return "se superpone con otra ciudad";
        }
    }
    for (const EditorDungeon& d: map.getDungeons()) {
        bool apart = originX + prefab.width <= d.x || d.x + d.width <= originX ||
                     originY + prefab.height <= d.y || d.y + d.height <= originY;
        if (!apart) {
            return "se superpone con una mazmorra";
        }
    }
    for (const EditorForest& f: map.getForests()) {
        bool apart = originX + prefab.width <= f.x || f.x + f.width <= originX ||
                     originY + prefab.height <= f.y || f.y + f.height <= originY;
        if (!apart) {
            return "se superpone con un bosque";
        }
    }
    return "";
}

void applyCityPrefab(EditorMap& map, int originX, int originY, const std::string& name) {
    const CityPrefab& prefab = getCityPrefab();
    for (const CityCell& c: prefab.ground) {
        map.setGround(originX + c.dx, originY + c.dy, c.value);
    }
    for (const CityCell& c: prefab.decoration) {
        map.setDecoration(originX + c.dx, originY + c.dy, c.value);
    }
    for (const CityCell& c: prefab.roofs) {
        map.setRoof(originX + c.dx, originY + c.dy, c.value);
    }
    for (const CityCell& c: prefab.indoor) {
        map.setIndoor(originX + c.dx, originY + c.dy, c.value);
    }
    for (const CityCell& c: prefab.obstacles) {
        map.addObstacle(originX + c.dx, originY + c.dy);
    }
    for (const CityNpc& npc: prefab.npcs) {
        map.addCitizen(npc.type, originX + npc.dx, originY + npc.dy);
    }
    map.addSafeZone(name, originX + prefab.safeDx, originY + prefab.safeDy, prefab.safeW,
                    prefab.safeH);
}

void clearCity(EditorMap& map, int originX, int originY) {
    const CityPrefab& prefab = getCityPrefab();
    for (const CityCell& c: prefab.ground) {
        map.setGround(originX + c.dx, originY + c.dy, GRASS);
    }
    for (const CityCell& c: prefab.decoration) {
        map.setDecoration(originX + c.dx, originY + c.dy, 0);
    }
    for (const CityCell& c: prefab.roofs) {
        map.setRoof(originX + c.dx, originY + c.dy, 0);
    }
    for (const CityCell& c: prefab.indoor) {
        map.setIndoor(originX + c.dx, originY + c.dy, 0);
    }
    for (const CityCell& c: prefab.obstacles) {
        map.removeObstacle(originX + c.dx, originY + c.dy);
    }
    map.removeCitizensInRect(originX, originY, prefab.width, prefab.height);
    map.removeSafeZoneAt(originX + prefab.safeDx, originY + prefab.safeDy);
}

bool eraseCityAt(EditorMap& map, int col, int row) {
    const EditorSafeZone* zone = zoneContaining(map, col, row);
    if (zone == nullptr) {
        return false;
    }
    const CityPrefab& prefab = getCityPrefab();
    clearCity(map, zone->x - prefab.safeDx, zone->y - prefab.safeDy);
    return true;
}

CellPos cityOriginForClick(int col, int row) {
    const CityPrefab& prefab = getCityPrefab();
    return {col - prefab.width / 2, row - prefab.height / 2};
}

bool cityZoneFor(const EditorSafeZone& zone, const std::string& citizenType, CellRect& out) {
    const std::string zoneName = zoneNameForCitizen(citizenType);
    if (zoneName.empty()) {
        return false;
    }
    const CityPrefab& prefab = getCityPrefab();
    auto it = std::find_if(prefab.buildings.begin(), prefab.buildings.end(),
                           [&zoneName](const CityZone& z) { return z.name == zoneName; });
    if (it == prefab.buildings.end()) {
        return false;
    }
    out = {zone.x - prefab.safeDx + it->dx, zone.y - prefab.safeDy + it->dy, it->width, it->height};
    return true;
}

std::string citizenPlacementError(const EditorMap& map, const std::string& citizenType, int col,
                                  int row) {
    const EditorSafeZone* zone = zoneContaining(map, col, row);
    if (zone == nullptr) {
        return "los citizens van dentro de una ciudad";
    }
    if (map.isBlocked(col, row)) {
        return "esa celda esta ocupada";
    }
    CellRect rect;
    if (!cityZoneFor(*zone, citizenType, rect)) {
        return buildingErrorFor(citizenType);
    }
    bool inside = col >= rect.x && col < rect.x + rect.width && row >= rect.y &&
                  row < rect.y + rect.height;
    if (!inside) {
        return buildingErrorFor(citizenType);
    }
    return "";
}

std::string monsterPlacementError(const EditorMap& map, int col, int row) {
    if (zoneContaining(map, col, row) != nullptr) {
        return "no se pueden poner monstruos en la ciudad";
    }
    for (const EditorDungeon& dungeon: map.getDungeons()) {
        if (col >= dungeon.x && col < dungeon.x + dungeon.width && row >= dungeon.y &&
            row < dungeon.y + dungeon.height) {
            return "no se pueden poner monstruos en la mazmorra";
        }
    }
    return "";
}
