#ifndef EDITOR_MAP_H
#define EDITOR_MAP_H

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "common/utils/position.h"

class EditorMap {
private:
    int width;
    int height;
    int tileSize;
    int tilesetCols;
    std::string tileset;
    std::vector<std::vector<int>> tiles;
    Position spawnPos;

    // Metadata retention
    nlohmann::json safeZonesData;
    nlohmann::json npcsData;

public:
    EditorMap(int width, int height, int tileSize, const std::string& tileset, int tilesetCols);
    explicit EditorMap(const std::string& jsonText);

    std::string toJson() const;

    int tileAt(int col, int row) const;
    void setTile(int col, int row, int tileId);

    Position getSpawn() const;
    void setSpawn(int col, int row);

    void resize(int newWidth, int newHeight);

    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;
    int getTilesetCols() const;
    const std::string& getTileset() const;
};

#endif
