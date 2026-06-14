#ifndef EDITOR_MAP_H
#define EDITOR_MAP_H

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

struct PlacedItem {
    int overlayIndex;
    int amount;
};

class EditorMap {
private:
    int width;
    int height;
    int tileSize;
    int tilesetCols;
    std::string tileset;
    std::vector<std::vector<int>> ground;
    std::vector<std::vector<int>> ground2;
    std::vector<std::vector<int>> decoration;
    std::vector<std::vector<int>> roofs;
    std::vector<std::vector<int>> indoor;
    std::vector<std::pair<int, int>> obstacles;
    Position spawnPos;
    std::vector<EditorSafeZone> safeZones;
    std::vector<CitizenSpawn> citizens;
    std::vector<MonsterSpawn> monsters;
    std::map<std::pair<int, int>, PlacedItem> items;
    nlohmann::json extraItems = nlohmann::json::array();

    bool inside(int col, int row) const;

public:
    EditorMap(int width, int height, int tileSize, const std::string& tileset, int tilesetCols);
    explicit EditorMap(const std::string& jsonText);

    std::string toJson() const;

    const std::vector<std::vector<int>>& getGround() const;
    const std::vector<std::vector<int>>& getGround2() const;
    const std::vector<std::vector<int>>& getDecoration() const;
    const std::vector<std::vector<int>>& getRoofs() const;
    const std::vector<std::vector<int>>& getIndoor() const;

    void setGround(int col, int row, int value);
    void setDecoration(int col, int row, int value);
    void setRoof(int col, int row, int value);
    void setIndoor(int col, int row, int value);

    void addObstacle(int col, int row);
    void removeObstacle(int col, int row);
    bool isBlocked(int col, int row) const;

    void paintItem(int col, int row, int overlayIndex);
    const PlacedItem* itemAt(int col, int row) const;
    void removeItemAt(int col, int row);
    const std::map<std::pair<int, int>, PlacedItem>& getItems() const;

    Position getSpawn() const;
    void setSpawn(int col, int row);

    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;
    int getTilesetCols() const;
    const std::string& getTileset() const;

    const std::vector<EditorSafeZone>& getSafeZones() const;
    void addSafeZone(const std::string& name, int x, int y, int width, int height);
    void removeSafeZoneAt(int x, int y);

    const std::vector<CitizenSpawn>& getCitizens() const;
    void addCitizen(const std::string& type, int x, int y);
    void removeCitizensInRect(int x, int y, int width, int height);
    void removeEntitiesAt(int x, int y);

    const std::vector<MonsterSpawn>& getMonsters() const;
    void addMonster(const std::string& type, int x, int y);
};

#endif
