#include "BlockBrush.h"

#include "BeachStamp.h"
#include "CityStamp.h"
#include "DesertStamp.h"
#include "DungeonStamp.h"
#include "ForestStamp.h"

std::string blockStampError(const EditorMap& map, TerrainBlock block, int col, int row) {
    switch (block) {
        case TerrainBlock::CITY: {
            CellPos o = cityOriginForClick(col, row);
            return cityStampError(map, o.x, o.y);
        }
        case TerrainBlock::FOREST: {
            CellPos o = forestOriginForClick(col, row);
            return forestStampError(map, o.x, o.y);
        }
        case TerrainBlock::DUNGEON: {
            CellPos o = dungeonOriginForClick(col, row);
            return dungeonStampError(map, o.x, o.y);
        }
        case TerrainBlock::DESERT: {
            CellPos o = desertOriginForClick(col, row);
            return desertStampError(map, o.x, o.y);
        }
        case TerrainBlock::BEACH: {
            CellPos o = beachOriginForClick(col, row);
            return beachStampError(map, o.x, o.y);
        }
        case TerrainBlock::NONE:
            return "";
    }
    return "";
}

void applyBlock(EditorMap& map, TerrainBlock block, int col, int row) {
    switch (block) {
        case TerrainBlock::CITY: {
            CellPos o = cityOriginForClick(col, row);
            applyCityPrefab(map, o.x, o.y,
                            "Ciudad " + std::to_string(map.getSafeZones().size() + 1));
            break;
        }
        case TerrainBlock::FOREST: {
            CellPos o = forestOriginForClick(col, row);
            applyForestPrefab(map, o.x, o.y);
            break;
        }
        case TerrainBlock::DUNGEON: {
            CellPos o = dungeonOriginForClick(col, row);
            applyDungeonPrefab(map, o.x, o.y);
            break;
        }
        case TerrainBlock::DESERT: {
            CellPos o = desertOriginForClick(col, row);
            applyDesertPrefab(map, o.x, o.y);
            break;
        }
        case TerrainBlock::BEACH: {
            CellPos o = beachOriginForClick(col, row);
            applyBeachPrefab(map, o.x, o.y);
            break;
        }
        case TerrainBlock::NONE:
            break;
    }
}
