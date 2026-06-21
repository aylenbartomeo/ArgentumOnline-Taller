#ifndef CITY_STAMP_H
#define CITY_STAMP_H

#include <string>

#include "EditorMap.h"

struct CellRect {
    int x;
    int y;
    int width;
    int height;
};

struct CellPos {
    int x;
    int y;
};

bool cityZoneFor(const EditorSafeZone& zone, const std::string& citizenType, CellRect& out);
std::string citizenPlacementError(const EditorMap& map, const std::string& citizenType, int col,
                                  int row);
std::string monsterPlacementError(const EditorMap& map, const std::string& monsterType, int col,
                                  int row);
std::string itemPlacementError(const EditorMap& map, int col, int row);

std::string cityStampError(const EditorMap& map, int originX, int originY);
void applyCityPrefab(EditorMap& map, int originX, int originY, const std::string& name);
void clearCity(EditorMap& map, int originX, int originY);
bool eraseCityAt(EditorMap& map, int col, int row);
CellPos cityOriginForClick(int col, int row);

#endif
