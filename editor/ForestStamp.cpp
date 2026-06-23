#include "ForestStamp.h"

#include <algorithm>

#include "ForestPrefab.h"

namespace {
const EditorForest* forestContaining(const EditorMap& map, int col, int row) {
    const auto& forests = map.getForests();
    auto it = std::find_if(forests.begin(), forests.end(), [col, row](const EditorForest& f) {
        return col >= f.x && col < f.x + f.width && row >= f.y && row < f.y + f.height;
    });
    return (it != forests.end()) ? &(*it) : nullptr;
}
}  // namespace

std::string forestStampError(const EditorMap& map, int originX, int originY) {
    const ForestPrefab& prefab = getForestPrefab();
    if (originX < 0 || originY < 0 || originX + prefab.width > map.getWidth() ||
        originY + prefab.height > map.getHeight()) {
        return "el bosque no entra en el mapa";
    }
    for (const EditorSafeZone& zone: map.getSafeZones()) {
        bool apart = originX + prefab.width <= zone.x || zone.x + zone.width <= originX ||
                     originY + prefab.height <= zone.y || zone.y + zone.height <= originY;
        if (!apart) {
            return "se superpone con una ciudad";
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
            return "se superpone con otro bosque";
        }
    }
    return "";
}

void applyForestPrefab(EditorMap& map, int originX, int originY) {
    const ForestPrefab& prefab = getForestPrefab();
    for (const ForestCell& c: prefab.decoration) {
        map.setDecoration(originX + c.dx, originY + c.dy, c.value);
    }
    for (const ForestCell& c: prefab.obstacles) {
        map.addObstacle(originX + c.dx, originY + c.dy);
    }
    map.addForest(originX, originY, prefab.width, prefab.height);
}

void clearForest(EditorMap& map, int originX, int originY) {
    const ForestPrefab& prefab = getForestPrefab();
    for (const ForestCell& c: prefab.decoration) {
        map.setDecoration(originX + c.dx, originY + c.dy, 0);
    }
    for (const ForestCell& c: prefab.obstacles) {
        map.removeObstacle(originX + c.dx, originY + c.dy);
    }
    map.removeForestAt(originX, originY);
}

bool eraseForestAt(EditorMap& map, int col, int row) {
    const EditorForest* forest = forestContaining(map, col, row);
    if (forest == nullptr) {
        return false;
    }
    clearForest(map, forest->x, forest->y);
    return true;
}

CellPos forestOriginForClick(int col, int row) {
    const ForestPrefab& prefab = getForestPrefab();
    return {col - prefab.width / 2, row - prefab.height / 2};
}
