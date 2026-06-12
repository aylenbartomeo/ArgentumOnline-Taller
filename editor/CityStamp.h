#ifndef CITY_STAMP_H
#define CITY_STAMP_H

#include <string>

#include "EditorMap.h"

std::string cityStampError(const EditorMap& map, int originX, int originY);
void applyCityPrefab(EditorMap& map, int originX, int originY, const std::string& name);

#endif
