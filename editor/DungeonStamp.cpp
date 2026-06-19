#include "DungeonStamp.h"

#include <algorithm>

#include "DungeonPrefab.h"
#include "OverlayRegistry.h"

namespace {
constexpr int GRASS = 108;

int overlayIndexForItemId(int itemId) {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    for (size_t i = 0; i < registry.size(); ++i) {
        if (registry[i].itemId == itemId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

const EditorDungeon* dungeonContaining(const EditorMap& map, int col, int row) {
    const auto& dungeons = map.getDungeons();
    auto it = std::find_if(dungeons.begin(), dungeons.end(), [col, row](const EditorDungeon& d) {
        return col >= d.x && col < d.x + d.width && row >= d.y && row < d.y + d.height;
    });
    return (it != dungeons.end()) ? &(*it) : nullptr;
}
}  // namespace

std::string dungeonStampError(const EditorMap& map, int originX, int originY) {
    const DungeonPrefab& prefab = getDungeonPrefab();
    if (originX < 0 || originY < 0 || originX + prefab.width > map.getWidth() ||
        originY + prefab.height > map.getHeight()) {
        return "la mazmorra no entra en el mapa";
    }
    for (const EditorSafeZone& zone: map.getSafeZones()) {
        bool apart = originX + prefab.width <= zone.x || zone.x + zone.width <= originX ||
                     originY + prefab.height <= zone.y || zone.y + zone.height <= originY;
        if (!apart) {
            return "se superpone con una ciudad";
        }
    }
    for (const EditorDungeon& d: map.getDungeons()) {
        int ox = d.x - prefab.dungeonDx;
        int oy = d.y - prefab.dungeonDy;
        bool apart = originX + prefab.width <= ox || ox + prefab.width <= originX ||
                     originY + prefab.height <= oy || oy + prefab.height <= originY;
        if (!apart) {
            return "se superpone con otra mazmorra";
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

void applyDungeonPrefab(EditorMap& map, int originX, int originY) {
    const DungeonPrefab& prefab = getDungeonPrefab();
    for (const DungeonCell& c: prefab.ground) {
        map.setGround(originX + c.dx, originY + c.dy, c.value);
    }
    for (const DungeonCell& c: prefab.decoration) {
        map.setDecoration(originX + c.dx, originY + c.dy, c.value);
    }
    for (const DungeonCell& c: prefab.obstacles) {
        map.addObstacle(originX + c.dx, originY + c.dy);
    }
    for (const DungeonItem& g: prefab.gold) {
        int idx = overlayIndexForItemId(g.itemId);
        if (idx >= 0) {
            map.setItem(originX + g.dx, originY + g.dy, idx, g.amount);
        }
    }
    map.addDungeon(originX + prefab.dungeonDx, originY + prefab.dungeonDy, prefab.dungeonW,
                   prefab.dungeonH);
}

void clearDungeon(EditorMap& map, int originX, int originY) {
    const DungeonPrefab& prefab = getDungeonPrefab();
    for (const DungeonCell& c: prefab.ground) {
        map.setGround(originX + c.dx, originY + c.dy, GRASS);
    }
    for (const DungeonCell& c: prefab.decoration) {
        map.setDecoration(originX + c.dx, originY + c.dy, 0);
    }
    for (const DungeonCell& c: prefab.obstacles) {
        map.removeObstacle(originX + c.dx, originY + c.dy);
    }
    for (const DungeonItem& g: prefab.gold) {
        map.removeItemAt(originX + g.dx, originY + g.dy);
    }
    map.removeDungeonAt(originX + prefab.dungeonDx, originY + prefab.dungeonDy);
}

bool eraseDungeonAt(EditorMap& map, int col, int row) {
    const EditorDungeon* dungeon = dungeonContaining(map, col, row);
    if (dungeon == nullptr) {
        return false;
    }
    const DungeonPrefab& prefab = getDungeonPrefab();
    clearDungeon(map, dungeon->x - prefab.dungeonDx, dungeon->y - prefab.dungeonDy);
    return true;
}

CellPos dungeonOriginForClick(int col, int row) {
    const DungeonPrefab& prefab = getDungeonPrefab();
    int arenaW = prefab.dungeonW + 2;
    int arenaH = prefab.dungeonH + 2;
    return {col - arenaW / 2, row - arenaH / 2};
}
