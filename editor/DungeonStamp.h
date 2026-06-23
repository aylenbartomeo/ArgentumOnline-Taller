#ifndef DUNGEON_STAMP_H
#define DUNGEON_STAMP_H

#include <string>

#include "CityStamp.h"
#include "EditorMap.h"

std::string dungeonStampError(const EditorMap& map, int originX, int originY);
void applyDungeonPrefab(EditorMap& map, int originX, int originY);
void clearDungeon(EditorMap& map, int originX, int originY);
bool eraseDungeonAt(EditorMap& map, int col, int row);
CellPos dungeonOriginForClick(int col, int row);

#endif
