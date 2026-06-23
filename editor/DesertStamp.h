#ifndef DESERT_STAMP_H
#define DESERT_STAMP_H

#include <string>

#include "CityStamp.h"
#include "EditorMap.h"

std::string desertStampError(const EditorMap& map, int originX, int originY);
void applyDesertPrefab(EditorMap& map, int originX, int originY);
void clearDesert(EditorMap& map, int originX, int originY);
bool eraseDesertAt(EditorMap& map, int col, int row);
CellPos desertOriginForClick(int col, int row);

#endif
