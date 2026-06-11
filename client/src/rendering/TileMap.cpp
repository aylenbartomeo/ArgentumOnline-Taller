#include "TileMap.h"

#include <algorithm>
#include <iterator>
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

    if (data.contains("terrain")) {
        terrain = data.at("terrain").get<std::vector<std::vector<int>>>();
    } else {
        terrain.assign(height, std::vector<int>(width, 0));
    }

    auto loadGrid = [&data, this](const char* key) {
        if (data.contains(key)) {
            return data.at(key).get<std::vector<std::vector<int>>>();
        }
        return std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
    };
    ground = loadGrid("ground");
    ground2 = loadGrid("ground2");
    decoration = loadGrid("decoration");
    roofs = loadGrid("roofs");
    indoor = loadGrid("indoor");

    if (data.contains("safeZones")) {
        const auto& zones = data.at("safeZones");
        std::transform(zones.begin(), zones.end(), std::back_inserter(safeZones),
                       [](const nlohmann::json& zone) {
                           return SafeZoneRect{zone.at("x").get<int>(), zone.at("y").get<int>(),
                                               zone.at("width").get<int>(),
                                               zone.at("height").get<int>()};
                       });
    }

    if (data.contains("npcs")) {
        const auto& npcs = data.at("npcs");
        std::transform(npcs.begin(), npcs.end(), std::back_inserter(citizens),
                       [](const nlohmann::json& npc) {
                           return MapCitizen{npc.at("type").get<std::string>(),
                                             npc.at("x").get<int>(), npc.at("y").get<int>()};
                       });
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

int TileMap::terrainAt(int col, int row) const { return terrain.at(row).at(col); }

const std::vector<std::vector<int>>& TileMap::getGround() const { return ground; }
const std::vector<std::vector<int>>& TileMap::getGround2() const { return ground2; }
const std::vector<std::vector<int>>& TileMap::getDecoration() const { return decoration; }
const std::vector<std::vector<int>>& TileMap::getRoofs() const { return roofs; }
const std::vector<std::vector<int>>& TileMap::getIndoor() const { return indoor; }
