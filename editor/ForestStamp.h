#ifndef FOREST_STAMP_H
#define FOREST_STAMP_H

#include <string>

#include "CityStamp.h"
#include "EditorMap.h"

std::string forestStampError(const EditorMap& map, int originX, int originY);
void applyForestPrefab(EditorMap& map, int originX, int originY);
void clearForest(EditorMap& map, int originX, int originY);
bool eraseForestAt(EditorMap& map, int col, int row);
CellPos forestOriginForClick(int col, int row);

#endif
