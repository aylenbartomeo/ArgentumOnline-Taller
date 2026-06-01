#include "TileMap.h"

#include <algorithm>
#include <stdexcept>

#include <nlohmann/json.hpp>

TileMap::TileMap(const std::string& jsonText) {
    nlohmann::json data = nlohmann::json::parse(jsonText);

    width = data.at("width").get<int>();
    height = data.at("height").get<int>();
    tileSize = data.at("tileSize").get<int>();
    tilesetCols = data.at("tilesetCols").get<int>();
    tileset = data.at("tileset").get<std::string>();
    tiles = data.at("tiles").get<std::vector<std::vector<int>>>();

    if (static_cast<int>(tiles.size()) != height) {
        throw std::runtime_error("TileMap: la cantidad de filas no coincide con height");
    }
    if (std::any_of(tiles.begin(), tiles.end(), [this](const std::vector<int>& row) {
            return static_cast<int>(row.size()) != width;
        })) {
        throw std::runtime_error("TileMap: una fila no coincide con width");
    }

    if (data.contains("safeZones")) {
        for (const auto& zone: data.at("safeZones")) {
            safeZones.push_back({zone.at("x").get<int>(), zone.at("y").get<int>(),
                                 zone.at("width").get<int>(), zone.at("height").get<int>()});
        }
    }

    if (data.contains("npcs")) {
        for (const auto& npc: data.at("npcs")) {
            citizens.push_back({npc.at("type").get<std::string>(), npc.at("x").get<int>(),
                                npc.at("y").get<int>()});
        }
    }
}

int TileMap::getWidth() const { return width; }
int TileMap::getHeight() const { return height; }
int TileMap::getTileSize() const { return tileSize; }
int TileMap::getTilesetCols() const { return tilesetCols; }
const std::string& TileMap::getTileset() const { return tileset; }
const std::vector<SafeZoneRect>& TileMap::getSafeZones() const { return safeZones; }
const std::vector<MapCitizen>& TileMap::getCitizens() const { return citizens; }

int TileMap::tileAt(int col, int row) const { return tiles.at(row).at(col); }
