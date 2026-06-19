#ifndef BLOCK_BRUSH_H
#define BLOCK_BRUSH_H

#include <string>

#include "EditorMap.h"

enum class TerrainBlock { NONE, CITY, FOREST, DUNGEON, BEACH, DESERT };

std::string blockStampError(const EditorMap& map, TerrainBlock block, int col, int row);
void applyBlock(EditorMap& map, TerrainBlock block, int col, int row);

#endif
