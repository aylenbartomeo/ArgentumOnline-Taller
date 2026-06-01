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

    if (data.contains("safeZones")) {
        for (const auto& zone: data.at("safeZones")) {
            EditorSafeZone safeZone;
            safeZone.name = zone.value("name", std::string(""));
            safeZone.x = zone.at("x").get<int>();
            safeZone.y = zone.at("y").get<int>();
            safeZone.width = zone.at("width").get<int>();
            safeZone.height = zone.at("height").get<int>();
            safeZones.push_back(safeZone);
        }
    }

    if (data.contains("npcs")) {
        for (const auto& npc: data.at("npcs")) {
            citizens.push_back({npc.at("type").get<std::string>(), npc.at("x").get<int>(),
                                npc.at("y").get<int>()});
        }
    }

    if (data.contains("monsters")) {
        for (const auto& monster: data.at("monsters")) {
            monsters.push_back({monster.at("type").get<std::string>(), monster.at("x").get<int>(),
                                monster.at("y").get<int>()});
        }
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

    if (!safeZones.empty()) {
        nlohmann::json zonesJson = nlohmann::json::array();
        for (const auto& zone: safeZones) {
            nlohmann::json zoneJson = {
                    {"x", zone.x}, {"y", zone.y}, {"width", zone.width}, {"height", zone.height}};
            if (!zone.name.empty()) {
                zoneJson["name"] = zone.name;
            }
            zonesJson.push_back(zoneJson);
        }
        data["safeZones"] = zonesJson;
    }

    if (!citizens.empty()) {
        nlohmann::json citizensJson = nlohmann::json::array();
        for (const auto& citizen: citizens) {
            citizensJson.push_back({{"type", citizen.type}, {"x", citizen.x}, {"y", citizen.y}});
        }
        data["npcs"] = citizensJson;
    }

    if (!monsters.empty()) {
        nlohmann::json monstersJson = nlohmann::json::array();
        for (const auto& monster: monsters) {
            monstersJson.push_back({{"type", monster.type}, {"x", monster.x}, {"y", monster.y}});
        }
        data["monsters"] = monstersJson;
    }

<<<<<<< HEAD
    if (!items.empty()) {
        nlohmann::json itemsJson = nlohmann::json::array();
        for (const auto& item: items) {
            itemsJson.push_back({{"itemId", item.itemId}, {"x", item.x}, {"y", item.y}});
        }
        data["items"] = itemsJson;
    }

=======
>>>>>>> develop
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

const std::vector<EditorSafeZone>& EditorMap::getSafeZones() const { return safeZones; }

const std::vector<CitizenSpawn>& EditorMap::getCitizens() const { return citizens; }

void EditorMap::addCitizen(const std::string& type, int x, int y) {
    citizens.push_back({type, x, y});
}

const std::vector<MonsterSpawn>& EditorMap::getMonsters() const { return monsters; }

void EditorMap::addMonster(const std::string& type, int x, int y) {
    monsters.push_back({type, x, y});
}

void EditorMap::removeEntitiesAt(int x, int y) {
    auto matches = [x, y](auto& v) {
        v.erase(std::remove_if(v.begin(), v.end(),
                               [x, y](const auto& e) { return e.x == x && e.y == y; }),
                v.end());
    };
    matches(citizens);
    matches(monsters);
}
