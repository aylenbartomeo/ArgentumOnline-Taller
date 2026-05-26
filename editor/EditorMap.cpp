#include "EditorMap.h"

#include <algorithm>
#include <stdexcept>

#include <nlohmann/json.hpp>

EditorMap::EditorMap(int width, int height, int tileSize, const std::string& tileset,
                     int tilesetCols):
        width(width),
        height(height),
        tileSize(tileSize),
        tilesetCols(tilesetCols),
        tileset(tileset),
        tiles(height, std::vector<int>(width, 0)),
        spawnPos({0, 0}) {}

EditorMap::EditorMap(const std::string& jsonText) {
    nlohmann::json data = nlohmann::json::parse(jsonText);

    width = data.at("width").get<int>();
    height = data.at("height").get<int>();
    tileSize = data.at("tileSize").get<int>();
    tilesetCols = data.at("tilesetCols").get<int>();
    tileset = data.at("tileset").get<std::string>();
    tiles = data.at("tiles").get<std::vector<std::vector<int>>>();

    if (static_cast<int>(tiles.size()) != height) {
        throw std::runtime_error("EditorMap: la cantidad de filas no coincide con height");
    }
    if (std::any_of(tiles.begin(), tiles.end(),
                    [this](const auto& row) { return static_cast<int>(row.size()) != width; })) {
        throw std::runtime_error("EditorMap: una fila no coincide con width");
    }

    if (data.contains("spawn")) {
        spawnPos.x = data.at("spawn").at("x").get<int>();
        spawnPos.y = data.at("spawn").at("y").get<int>();
    } else {
        spawnPos = {0, 0};
    }
}

std::string EditorMap::toJson() const {
    nlohmann::json data;
    data["tileSize"] = tileSize;
    data["tileset"] = tileset;
    data["tilesetCols"] = tilesetCols;
    data["width"] = width;
    data["height"] = height;
    data["spawn"] = {{"x", spawnPos.x}, {"y", spawnPos.y}};
    data["tiles"] = tiles;
    return data.dump(4);
}

int EditorMap::tileAt(int col, int row) const { return tiles.at(row).at(col); }

void EditorMap::setTile(int col, int row, int tileId) { tiles.at(row).at(col) = tileId; }

Position EditorMap::getSpawn() const { return spawnPos; }

void EditorMap::setSpawn(int col, int row) {
    spawnPos.x = col;
    spawnPos.y = row;
}

void EditorMap::resize(int newWidth, int newHeight) {
    if (newWidth < 1 || newHeight < 1) {
        throw std::runtime_error("EditorMap: dimensiones inválidas en resize");
    }
    tiles.resize(newHeight, std::vector<int>(newWidth, 0));
    for (auto& row: tiles) {
        row.resize(newWidth, 0);
    }
    width = newWidth;
    height = newHeight;
    if (spawnPos.x >= width) {
        spawnPos.x = width - 1;
    }
    if (spawnPos.y >= height) {
        spawnPos.y = height - 1;
    }
}

int EditorMap::getWidth() const { return width; }
int EditorMap::getHeight() const { return height; }
int EditorMap::getTileSize() const { return tileSize; }
int EditorMap::getTilesetCols() const { return tilesetCols; }
const std::string& EditorMap::getTileset() const { return tileset; }
