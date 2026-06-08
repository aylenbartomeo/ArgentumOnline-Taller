#ifndef EDITOR_MAP_H
#define EDITOR_MAP_H

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "common/utils/position.h"

struct EditorSafeZone {
    std::string name;
    int x;
    int y;
    int width;
    int height;
};

struct CitizenSpawn {
    std::string type;
    int x;
    int y;
};

struct MonsterSpawn {
    std::string type;
    int x;
    int y;
};

class EditorMap {
private:
    int width;
    int height;
    int tileSize;
    int tilesetCols;
    std::string tileset;
    std::vector<std::vector<int>> tiles;
    Position spawnPos;
    std::vector<EditorSafeZone> safeZones;
    std::vector<CitizenSpawn> citizens;
    std::vector<MonsterSpawn> monsters;
    std::map<std::pair<int, int>, int> overlayAmounts;

public:
    EditorMap(int width, int height, int tileSize, const std::string& tileset, int tilesetCols);
    explicit EditorMap(const std::string& jsonText);

    std::string toJson() const;

    int tileAt(int col, int row) const;
    void setTile(int col, int row, int tileId);
    void paintOverlay(int col, int row, int overlayIndex);

    Position getSpawn() const;
    void setSpawn(int col, int row);

    void resize(int newWidth, int newHeight);

    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;
    int getTilesetCols() const;
    const std::string& getTileset() const;

    const std::vector<EditorSafeZone>& getSafeZones() const;

    const std::vector<CitizenSpawn>& getCitizens() const;
    void addCitizen(const std::string& type, int x, int y);
    void removeEntitiesAt(int x, int y);

    const std::vector<MonsterSpawn>& getMonsters() const;
    void addMonster(const std::string& type, int x, int y);
};

#endif
