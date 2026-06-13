#include "CityStamp.h"

#include <vector>

#include "CityPrefab.h"

namespace {
constexpr int WATER = 109;
constexpr int GRASS = 108;
}

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
    map.addSafeZone(name, originX, originY, prefab.width, prefab.height);
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
    map.removeSafeZoneAt(originX, originY);
}
