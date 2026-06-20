#ifndef BEACH_STAMP_H
#define BEACH_STAMP_H

#include <string>

#include "CityStamp.h"
#include "EditorMap.h"

std::string beachStampError(const EditorMap& map, int originX, int originY);
void applyBeachPrefab(EditorMap& map, int originX, int originY);
void clearBeach(EditorMap& map, int originX, int originY);
bool eraseBeachAt(EditorMap& map, int col, int row);
CellPos beachOriginForClick(int col, int row);

#endif
