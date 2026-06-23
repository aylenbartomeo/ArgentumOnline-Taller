#include "SmartEraser.h"

#include <algorithm>

#include "BeachStamp.h"
#include "CityStamp.h"
#include "DesertStamp.h"
#include "DungeonStamp.h"
#include "ForestStamp.h"

namespace {
bool hasMonsterAt(const EditorMap& map, int col, int row) {
    const auto& monsters = map.getMonsters();
    return std::any_of(monsters.begin(), monsters.end(),
                       [col, row](const MonsterSpawn& m) { return m.x == col && m.y == row; });
}

bool hasCitizenAt(const EditorMap& map, int col, int row) {
    const auto& citizens = map.getCitizens();
    return std::any_of(citizens.begin(), citizens.end(),
                       [col, row](const CitizenSpawn& c) { return c.x == col && c.y == row; });
}

bool contains(int x, int y, int w, int h, int col, int row) {
    return col >= x && col < x + w && row >= y && row < y + h;
}

BlockKind blockAt(const EditorMap& map, int col, int row) {
    const auto& safeZones = map.getSafeZones();
    if (std::any_of(safeZones.begin(), safeZones.end(), [col, row](const EditorSafeZone& z) {
            return contains(z.x, z.y, z.width, z.height, col, row);
        })) {
        return BlockKind::CITY;
    }
    const auto& dungeons = map.getDungeons();
    if (std::any_of(dungeons.begin(), dungeons.end(), [col, row](const EditorDungeon& d) {
            return contains(d.x, d.y, d.width, d.height, col, row);
        })) {
        return BlockKind::DUNGEON;
    }
    const auto& forests = map.getForests();
    if (std::any_of(forests.begin(), forests.end(), [col, row](const EditorForest& f) {
            return contains(f.x, f.y, f.width, f.height, col, row);
        })) {
        return BlockKind::FOREST;
    }
    const auto& deserts = map.getDeserts();
    if (std::any_of(deserts.begin(), deserts.end(), [col, row](const EditorDesert& d) {
            return contains(d.x, d.y, d.width, d.height, col, row);
        })) {
        return BlockKind::DESERT;
    }
    const auto& beaches = map.getBeaches();
    if (std::any_of(beaches.begin(), beaches.end(), [col, row](const EditorBeach& b) {
            return contains(b.x, b.y, b.width, b.height, col, row);
        })) {
        return BlockKind::BEACH;
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
            } else if (target.block == BlockKind::DESERT) {
                eraseDesertAt(map, col, row);
            } else if (target.block == BlockKind::BEACH) {
                eraseBeachAt(map, col, row);
            }
            break;
        case EraseLayer::NONE:
            break;
    }
}
