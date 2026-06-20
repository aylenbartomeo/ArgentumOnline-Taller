#include "BeachStamp.h"

#include <algorithm>

#include "BeachPrefab.h"

namespace {
constexpr int GRASS = 108;

const EditorBeach* beachContaining(const EditorMap& map, int col, int row) {
    const auto& beaches = map.getBeaches();
    auto it = std::find_if(beaches.begin(), beaches.end(), [col, row](const EditorBeach& b) {
        return col >= b.x && col < b.x + b.width && row >= b.y && row < b.y + b.height;
    });
    return (it != beaches.end()) ? &(*it) : nullptr;
}
}  // namespace

std::string beachStampError(const EditorMap& map, int originX, int originY) {
    const BeachPrefab& prefab = getBeachPrefab();
    if (originX < 0 || originY < 0 || originX + prefab.width > map.getWidth() ||
        originY + prefab.height > map.getHeight()) {
        return "la playa no entra en el mapa";
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
            return "se superpone con un bosque";
        }
    }
    for (const EditorDesert& d: map.getDeserts()) {
        bool apart = originX + prefab.width <= d.x || d.x + d.width <= originX ||
                     originY + prefab.height <= d.y || d.y + d.height <= originY;
        if (!apart) {
            return "se superpone con un desierto";
        }
    }
    for (const EditorBeach& b: map.getBeaches()) {
        bool apart = originX + prefab.width <= b.x || b.x + b.width <= originX ||
                     originY + prefab.height <= b.y || b.y + b.height <= originY;
        if (!apart) {
            return "se superpone con otra playa";
        }
    }
    return "";
}

void applyBeachPrefab(EditorMap& map, int originX, int originY) {
    const BeachPrefab& prefab = getBeachPrefab();
    for (const BeachCell& c: prefab.ground) {
        map.setGround(originX + c.dx, originY + c.dy, c.value);
    }
    for (const BeachCell& c: prefab.ground2) {
        map.setGround2(originX + c.dx, originY + c.dy, c.value);
    }
    for (const BeachCell& c: prefab.decoration) {
        map.setDecoration(originX + c.dx, originY + c.dy, c.value);
    }
    for (const BeachCell& c: prefab.obstacles) {
        map.addObstacle(originX + c.dx, originY + c.dy);
    }
    map.addBeach(originX, originY, prefab.width, prefab.height);
}

void clearBeach(EditorMap& map, int originX, int originY) {
    const BeachPrefab& prefab = getBeachPrefab();
    for (const BeachCell& c: prefab.ground) {
        map.setGround(originX + c.dx, originY + c.dy, GRASS);
    }
    for (const BeachCell& c: prefab.ground2) {
        map.setGround2(originX + c.dx, originY + c.dy, 0);
    }
    for (const BeachCell& c: prefab.decoration) {
        map.setDecoration(originX + c.dx, originY + c.dy, 0);
    }
    for (const BeachCell& c: prefab.obstacles) {
        map.removeObstacle(originX + c.dx, originY + c.dy);
    }
    map.removeBeachAt(originX, originY);
}

bool eraseBeachAt(EditorMap& map, int col, int row) {
    const EditorBeach* beach = beachContaining(map, col, row);
    if (beach == nullptr) {
        return false;
    }
    clearBeach(map, beach->x, beach->y);
    return true;
}

CellPos beachOriginForClick(int col, int row) {
    const BeachPrefab& prefab = getBeachPrefab();
    return {col - prefab.width / 2, row - prefab.height / 2};
}
