#ifndef TILEMAP_H
#define TILEMAP_H

#include <string>
#include <vector>

#include "SafeZoneRect.h"

struct MapCitizen {
    std::string type;
    int x;
    int y;
};

class TileMap {
private:
    int width;
    int height;
    int tileSize;
    int tilesetCols;
    std::string tileset;
    std::vector<std::vector<int>> tiles;
    std::vector<SafeZoneRect> safeZones;
    std::vector<MapCitizen> citizens;

public:
    explicit TileMap(const std::string& jsonText);

    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;
    int getTilesetCols() const;
    const std::string& getTileset() const;
    const std::vector<SafeZoneRect>& getSafeZones() const;
    const std::vector<MapCitizen>& getCitizens() const;

    int tileAt(int col, int row) const;
};

#endif
