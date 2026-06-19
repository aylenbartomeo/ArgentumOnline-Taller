#include "SmartEraser.h"

#include "CityStamp.h"
#include "DungeonStamp.h"
#include "ForestStamp.h"

namespace {
bool hasMonsterAt(const EditorMap& map, int col, int row) {
    for (const MonsterSpawn& m: map.getMonsters()) {
        if (m.x == col && m.y == row) {
            return true;
        }
    }
    return false;
}

bool hasCitizenAt(const EditorMap& map, int col, int row) {
    for (const CitizenSpawn& c: map.getCitizens()) {
        if (c.x == col && c.y == row) {
            return true;
        }
    }
    return false;
}

bool contains(int x, int y, int w, int h, int col, int row) {
    return col >= x && col < x + w && row >= y && row < y + h;
}

BlockKind blockAt(const EditorMap& map, int col, int row) {
    for (const EditorSafeZone& z: map.getSafeZones()) {
        if (contains(z.x, z.y, z.width, z.height, col, row)) {
            return BlockKind::CITY;
        }
    }
    for (const EditorDungeon& d: map.getDungeons()) {
        if (contains(d.x, d.y, d.width, d.height, col, row)) {
            return BlockKind::DUNGEON;
        }
    }
    for (const EditorForest& f: map.getForests()) {
        if (contains(f.x, f.y, f.width, f.height, col, row)) {
            return BlockKind::FOREST;
        }
    }
    return BlockKind::NONE;
}
}  // namespace

EraseTarget topErasableAt(const EditorMap& map, int col, int row) {
    if (hasMonsterAt(map, col, row)) {
        return {EraseLayer::MONSTER, BlockKind::NONE};
    }
    if (hasCitizenAt(map, col, row)) {
        return {EraseLayer::CITIZEN, BlockKind::NONE};
    }
    if (map.itemAt(col, row) != nullptr) {
        return {EraseLayer::ITEM, BlockKind::NONE};
    }
    BlockKind block = blockAt(map, col, row);
    if (block != BlockKind::NONE) {
        return {EraseLayer::BLOCK, block};
    }
    return {EraseLayer::NONE, BlockKind::NONE};
}

void smartEraseAt(EditorMap& map, int col, int row) {
    EraseTarget target = topErasableAt(map, col, row);
    switch (target.layer) {
        case EraseLayer::MONSTER:
            map.removeMonsterAt(col, row);
            break;
        case EraseLayer::CITIZEN:
            map.removeCitizenAt(col, row);
            break;
        case EraseLayer::ITEM:
            map.removeItemAt(col, row);
            break;
        case EraseLayer::BLOCK:
            if (target.block == BlockKind::CITY) {
                eraseCityAt(map, col, row);
            } else if (target.block == BlockKind::DUNGEON) {
                eraseDungeonAt(map, col, row);
            } else if (target.block == BlockKind::FOREST) {
                eraseForestAt(map, col, row);
            }
            break;
        case EraseLayer::NONE:
            break;
    }
}
