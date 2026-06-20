#include "DesertStamp.h"

#include <algorithm>

#include "DesertPrefab.h"

namespace {
constexpr int GRASS = 108;

const EditorDesert* desertContaining(const EditorMap& map, int col, int row) {
    const auto& deserts = map.getDeserts();
    auto it = std::find_if(deserts.begin(), deserts.end(), [col, row](const EditorDesert& d) {
        return col >= d.x && col < d.x + d.width && row >= d.y && row < d.y + d.height;
    });
    return (it != deserts.end()) ? &(*it) : nullptr;
}
}  // namespace

std::string desertStampError(const EditorMap& map, int originX, int originY) {
    const DesertPrefab& prefab = getDesertPrefab();
    if (originX < 0 || originY < 0 || originX + prefab.width > map.getWidth() ||
        originY + prefab.height > map.getHeight()) {
        return "el desierto no entra en el mapa";
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
            return "se superpone con otro desierto";
        }
    }
    return "";
}

void applyDesertPrefab(EditorMap& map, int originX, int originY) {
    const DesertPrefab& prefab = getDesertPrefab();
    for (const DesertCell& c: prefab.ground) {
        map.setGround(originX + c.dx, originY + c.dy, c.value);
    }
    map.addDesert(originX, originY, prefab.width, prefab.height);
}

void clearDesert(EditorMap& map, int originX, int originY) {
    const DesertPrefab& prefab = getDesertPrefab();
    for (const DesertCell& c: prefab.ground) {
        map.setGround(originX + c.dx, originY + c.dy, GRASS);
    }
    map.removeDesertAt(originX, originY);
}

bool eraseDesertAt(EditorMap& map, int col, int row) {
    const EditorDesert* desert = desertContaining(map, col, row);
    if (desert == nullptr) {
        return false;
    }
    clearDesert(map, desert->x, desert->y);
    return true;
}

CellPos desertOriginForClick(int col, int row) {
    const DesertPrefab& prefab = getDesertPrefab();
    return {col - prefab.width / 2, row - prefab.height / 2};
}
