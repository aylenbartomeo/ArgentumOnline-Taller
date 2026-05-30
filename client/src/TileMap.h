#ifndef TILEMAP_H
#define TILEMAP_H

#include <string>
#include <vector>

#include "SafeZoneRect.h"

class TileMap {
private:
    int width;
    int height;
    int tileSize;
    int tilesetCols;
    std::string tileset;
    std::vector<std::vector<int>> tiles;
    std::vector<SafeZoneRect> safeZones;

public:
    explicit TileMap(const std::string& jsonText);

    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;
    int getTilesetCols() const;
    const std::string& getTileset() const;
    const std::vector<SafeZoneRect>& getSafeZones() const;

    int tileAt(int col, int row) const;
};

#endif
