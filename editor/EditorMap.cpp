#include "EditorMap.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "OverlayRegistry.h"

namespace {
constexpr int GRASS = 108;

int overlayIndexForItemId(int itemId) {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    for (size_t i = 0; i < registry.size(); ++i) {
        if (registry[i].itemId == itemId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}
}  // namespace

EditorMap::EditorMap(int width, int height, int tileSize, const std::string& tileset,
                     int tilesetCols):
        width(width),
        height(height),
        tileSize(tileSize),
        tilesetCols(tilesetCols),
        tileset(tileset),
        ground(height, std::vector<int>(width, GRASS)),
        ground2(height, std::vector<int>(width, 0)),
        decoration(height, std::vector<int>(width, 0)),
        roofs(height, std::vector<int>(width, 0)),
        indoor(height, std::vector<int>(width, 0)),
        spawnPos({0, 0}) {}

EditorMap::EditorMap(const std::string& jsonText) {
    nlohmann::json data = nlohmann::json::parse(jsonText);

    width = data.at("width").get<int>();
    height = data.at("height").get<int>();
    tileSize = data.at("tileSize").get<int>();
    tilesetCols = data.at("tilesetCols").get<int>();
    tileset = data.at("tileset").get<std::string>();

    auto loadGrid = [&data, this](const char* key, int fallback) {
        if (data.contains(key)) {
            auto grid = data.at(key).get<std::vector<std::vector<int>>>();
            if (static_cast<int>(grid.size()) != height ||
                std::any_of(grid.begin(), grid.end(), [this](const std::vector<int>& row) {
                    return static_cast<int>(row.size()) != width;
                })) {
                throw std::runtime_error("EditorMap: la capa no coincide con width/height");
            }
            return grid;
        }
        return std::vector<std::vector<int>>(height, std::vector<int>(width, fallback));
    };
    ground = loadGrid("ground", GRASS);
    ground2 = loadGrid("ground2", 0);
    decoration = loadGrid("decoration", 0);
    roofs = loadGrid("roofs", 0);
    indoor = loadGrid("indoor", 0);

    if (data.contains("obstacles")) {
        const auto& obsData = data.at("obstacles");
        std::transform(obsData.begin(), obsData.end(), std::back_inserter(obstacles),
                       [](const nlohmann::json& obs) {
                           return std::make_pair(obs.at("x").get<int>(), obs.at("y").get<int>());
                       });
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
        const auto& npcs = data.at("npcs");
        std::transform(npcs.begin(), npcs.end(), std::back_inserter(citizens),
                       [](const nlohmann::json& npc) {
                           return CitizenSpawn{npc.at("type").get<std::string>(),
                                               npc.at("x").get<int>(), npc.at("y").get<int>()};
                       });
    }

    if (data.contains("monsters")) {
        const auto& monstersData = data.at("monsters");
        std::transform(monstersData.begin(), monstersData.end(), std::back_inserter(monsters),
                       [](const nlohmann::json& monster) {
                           return MonsterSpawn{monster.at("type").get<std::string>(),
                                               monster.at("x").get<int>(),
                                               monster.at("y").get<int>()};
                       });
    }

    if (data.contains("dungeons")) {
        for (const auto& d: data.at("dungeons")) {
            dungeons.push_back(EditorDungeon{d.at("x").get<int>(), d.at("y").get<int>(),
                                             d.at("width").get<int>(), d.at("height").get<int>()});
        }
    }

    if (data.contains("forests")) {
        for (const auto& f: data.at("forests")) {
            forests.push_back(EditorForest{f.at("x").get<int>(), f.at("y").get<int>(),
                                           f.at("width").get<int>(), f.at("height").get<int>()});
        }
    }

    if (data.contains("items")) {
        for (const auto& item: data.at("items")) {
            int overlayIndex = overlayIndexForItemId(item.at("id").get<int>());
            if (overlayIndex >= 0) {
                int x = item.at("x").get<int>();
                int y = item.at("y").get<int>();
                items[{x, y}] = PlacedItem{overlayIndex, item.value("amount", 1)};
            } else {
                extraItems.push_back(item);
            }
        }
    }
}

std::string EditorMap::toJson() const {
    nlohmann::json data;
    data["width"] = width;
    data["height"] = height;
    data["tileSize"] = tileSize;
    data["tileset"] = tileset;
    data["tilesetCols"] = tilesetCols;
    data["spawn"] = {{"x", spawnPos.x}, {"y", spawnPos.y}};
    data["ground"] = ground;
    data["ground2"] = ground2;
    data["decoration"] = decoration;
    data["roofs"] = roofs;
    data["indoor"] = indoor;

    nlohmann::json obstaclesJson = nlohmann::json::array();
    std::transform(obstacles.begin(), obstacles.end(), std::back_inserter(obstaclesJson),
                   [](const std::pair<int, int>& obs) {
                       return nlohmann::json{{"x", obs.first}, {"y", obs.second}};
                   });
    data["obstacles"] = obstaclesJson;

    nlohmann::json citizensJson = nlohmann::json::array();
    std::transform(citizens.begin(), citizens.end(), std::back_inserter(citizensJson),
                   [](const CitizenSpawn& c) {
                       return nlohmann::json{{"type", c.type}, {"x", c.x}, {"y", c.y}};
                   });
    data["npcs"] = citizensJson;

    nlohmann::json monstersJson = nlohmann::json::array();
    std::transform(monsters.begin(), monsters.end(), std::back_inserter(monstersJson),
                   [](const MonsterSpawn& m) {
                       return nlohmann::json{{"type", m.type}, {"x", m.x}, {"y", m.y}};
                   });
    data["monsters"] = monstersJson;

    nlohmann::json zonesJson = nlohmann::json::array();
    for (const EditorSafeZone& zone: safeZones) {
        nlohmann::json zoneJson = {
                {"x", zone.x}, {"y", zone.y}, {"width", zone.width}, {"height", zone.height}};
        if (!zone.name.empty()) {
            zoneJson["name"] = zone.name;
        }
        zonesJson.push_back(zoneJson);
    }
    data["safeZones"] = zonesJson;

    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    nlohmann::json itemsJson = nlohmann::json::array();
    for (const auto& entry: items) {
        const PlacedItem& item = entry.second;
        itemsJson.push_back({{"id", registry[item.overlayIndex].itemId},
                             {"x", entry.first.first},
                             {"y", entry.first.second},
                             {"amount", item.amount}});
    }
    std::copy(extraItems.begin(), extraItems.end(), std::back_inserter(itemsJson));
    if (!itemsJson.empty()) {
        data["items"] = itemsJson;
    }

    if (!dungeons.empty()) {
        nlohmann::json dungeonsJson = nlohmann::json::array();
        for (const EditorDungeon& d: dungeons) {
            dungeonsJson.push_back(
                    {{"x", d.x}, {"y", d.y}, {"width", d.width}, {"height", d.height}});
        }
        data["dungeons"] = dungeonsJson;
    }

    if (!forests.empty()) {
        nlohmann::json forestsJson = nlohmann::json::array();
        for (const EditorForest& f: forests) {
            forestsJson.push_back(
                    {{"x", f.x}, {"y", f.y}, {"width", f.width}, {"height", f.height}});
        }
        data["forests"] = forestsJson;
    }

    return data.dump(4);
}

bool EditorMap::inside(int col, int row) const {
    return col >= 0 && col < width && row >= 0 && row < height;
}

const std::vector<std::vector<int>>& EditorMap::getGround() const { return ground; }
const std::vector<std::vector<int>>& EditorMap::getGround2() const { return ground2; }
const std::vector<std::vector<int>>& EditorMap::getDecoration() const { return decoration; }
const std::vector<std::vector<int>>& EditorMap::getRoofs() const { return roofs; }
const std::vector<std::vector<int>>& EditorMap::getIndoor() const { return indoor; }

void EditorMap::setGround(int col, int row, int value) {
    if (inside(col, row)) {
        ground[row][col] = value;
    }
}

void EditorMap::setDecoration(int col, int row, int value) {
    if (inside(col, row)) {
        decoration[row][col] = value;
    }
}

void EditorMap::setRoof(int col, int row, int value) {
    if (inside(col, row)) {
        roofs[row][col] = value;
    }
}

void EditorMap::setIndoor(int col, int row, int value) {
    if (inside(col, row)) {
        indoor[row][col] = value;
    }
}

void EditorMap::addObstacle(int col, int row) {
    if (inside(col, row)) {
        obstacles.emplace_back(col, row);
    }
}

void EditorMap::removeObstacle(int col, int row) {
    obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
                                   [col, row](const std::pair<int, int>& obstacle) {
                                       return obstacle.first == col && obstacle.second == row;
                                   }),
                    obstacles.end());
}

bool EditorMap::isBlocked(int col, int row) const {
    return std::any_of(obstacles.begin(), obstacles.end(),
                       [col, row](const std::pair<int, int>& obstacle) {
                           return obstacle.first == col && obstacle.second == row;
                       });
}

void EditorMap::paintItem(int col, int row, int overlayIndex) {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    if (!inside(col, row) || overlayIndex < 0 ||
        overlayIndex >= static_cast<int>(registry.size())) {
        return;
    }
    auto it = items.find({col, row});
    if (it != items.end() && it->second.overlayIndex == overlayIndex &&
        registry[overlayIndex].stackable) {
        it->second.amount += 1;
    } else {
        items[{col, row}] = PlacedItem{overlayIndex, 1};
    }
}

void EditorMap::setItem(int col, int row, int overlayIndex, int amount) {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    if (!inside(col, row) || overlayIndex < 0 ||
        overlayIndex >= static_cast<int>(registry.size())) {
        return;
    }
    items[{col, row}] = PlacedItem{overlayIndex, amount};
}

const PlacedItem* EditorMap::itemAt(int col, int row) const {
    auto it = items.find({col, row});
    return (it != items.end()) ? &it->second : nullptr;
}

void EditorMap::removeItemAt(int col, int row) { items.erase({col, row}); }

const std::map<std::pair<int, int>, PlacedItem>& EditorMap::getItems() const { return items; }

Position EditorMap::getSpawn() const { return spawnPos; }

void EditorMap::setSpawn(int col, int row) {
    spawnPos.x = col;
    spawnPos.y = row;
}

int EditorMap::getWidth() const { return width; }
int EditorMap::getHeight() const { return height; }
int EditorMap::getTileSize() const { return tileSize; }
int EditorMap::getTilesetCols() const { return tilesetCols; }
const std::string& EditorMap::getTileset() const { return tileset; }

const std::vector<EditorSafeZone>& EditorMap::getSafeZones() const { return safeZones; }

void EditorMap::addSafeZone(const std::string& name, int x, int y, int width, int height) {
    safeZones.push_back({name, x, y, width, height});
}

void EditorMap::removeSafeZoneAt(int x, int y) {
    safeZones.erase(
            std::remove_if(safeZones.begin(), safeZones.end(),
                           [x, y](const EditorSafeZone& z) { return z.x == x && z.y == y; }),
            safeZones.end());
}

const std::vector<CitizenSpawn>& EditorMap::getCitizens() const { return citizens; }

void EditorMap::addCitizen(const std::string& type, int x, int y) {
    citizens.push_back({type, x, y});
}

void EditorMap::removeCitizensInRect(int x, int y, int width, int height) {
    citizens.erase(std::remove_if(citizens.begin(), citizens.end(),
                                  [x, y, width, height](const CitizenSpawn& c) {
                                      return c.x >= x && c.x < x + width && c.y >= y &&
                                             c.y < y + height;
                                  }),
                   citizens.end());
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

void EditorMap::removeMonsterAt(int x, int y) {
    monsters.erase(std::remove_if(monsters.begin(), monsters.end(),
                                  [x, y](const MonsterSpawn& m) { return m.x == x && m.y == y; }),
                   monsters.end());
}

void EditorMap::removeCitizenAt(int x, int y) {
    citizens.erase(std::remove_if(citizens.begin(), citizens.end(),
                                  [x, y](const CitizenSpawn& c) { return c.x == x && c.y == y; }),
                   citizens.end());
}

const std::vector<EditorDungeon>& EditorMap::getDungeons() const { return dungeons; }

void EditorMap::addDungeon(int x, int y, int width, int height) {
    dungeons.push_back({x, y, width, height});
}

void EditorMap::removeDungeonAt(int x, int y) {
    dungeons.erase(std::remove_if(dungeons.begin(), dungeons.end(),
                                  [x, y](const EditorDungeon& d) { return d.x == x && d.y == y; }),
                   dungeons.end());
}

const std::vector<EditorForest>& EditorMap::getForests() const { return forests; }

void EditorMap::addForest(int x, int y, int width, int height) {
    forests.push_back({x, y, width, height});
}

void EditorMap::removeForestAt(int x, int y) {
    forests.erase(std::remove_if(forests.begin(), forests.end(),
                                 [x, y](const EditorForest& f) { return f.x == x && f.y == y; }),
                  forests.end());
}
