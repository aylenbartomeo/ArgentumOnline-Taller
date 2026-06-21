#include "Map.h"

#include <cmath>
#include <fstream>
#include <optional>
#include <utility>

#include <nlohmann/json.hpp>

// Map::Map(const std::string& toml_filepath) { (void)toml_filepath; }
Map::Map(const std::string& toml_filepath): width(100), height(100) {
    (void)toml_filepath;
    safeZones.addZone("default", 45, 45, 10, 10);
}

void Map::loadFromToml(const std::string& filepath) { (void)filepath; }

Map::Map(): width(100), height(100), spawn_point({50.0f, 50.0f}) {
    safeZones.addZone("default", 45, 45, 10, 10);
    // Inicializa la grilla por defecto limpia
    collisionLayer.resize(width, height);
    entityCollisionLayer.resize(width, height);
}

void Map::setDimensions(int w, int h) {
    this->width = w;
    this->height = h;
    // Redimensionamos la matriz de colisiones para que coincida con el nuevo tamaño
    collisionLayer.resize(w, h);
    entityCollisionLayer.resize(w, h);
}

void Map::setCitizenArea(int x, int y, int w, int h) {
    safeZones.clear();
    safeZones.addZone("default", x, y, w, h);
}

void Map::setObstacleInGrid(int cell_x, int cell_y, bool is_solid) {
    if (cell_x >= 0 && cell_x < width && cell_y >= 0 && cell_y < height) {
        // 1. Lo pintamos en la grilla usando la nueva capa
        collisionLayer.setSolid(cell_x, cell_y, is_solid);

        // 2. Si es sólido, lo insertamos en el vector de elementos del mapa
        if (is_solid) {
            MapElement nuevo_obstaculo;
            nuevo_obstaculo.type = MapElementType::OBSTACLE;
            nuevo_obstaculo.area = {cell_x, cell_y, 1, 1};
            mapElements.push_back(nuevo_obstaculo);
        }
    }
    generate_collision_grid();
}

std::pair<float, float> Map::getInitialPosition() { return this->spawn_point; }

void Map::setSpawnPoint(float x, float y) { this->spawn_point = {x, y}; }

bool Map::loadSpawnFromJson(const std::string& path) {
    return loadSpawnFromJson(path, MapLoadOptions{});
}

bool Map::loadSpawnFromJson(const std::string& path, const MapLoadOptions& options) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }
    nlohmann::json data;
    try {
        file >> data;
    } catch (const nlohmann::json::exception&) {
        return false;
    }
    if (!data.contains("spawn") || !data.contains("width") || !data.contains("height")) {
        return false;
    }
    setDimensions(data["width"].get<int>(), data["height"].get<int>());
    setSpawnPoint(static_cast<float>(data["spawn"]["x"].get<int>()),
                  static_cast<float>(data["spawn"]["y"].get<int>()));

    safeZones.clear();
    npcs.clear();
    monsterSpawns.clear();
    bossZones.clear();
    zoneRects.clear();
    mapElements.clear();
    groundItems.clear();

    if (data.contains("bossZones")) {
        for (const auto& bz: data["bossZones"]) {
            BossZoneConfig config;
            config.x = bz["x"].get<int>();
            config.y = bz["y"].get<int>();
            config.width = bz["width"].get<int>();
            config.height = bz["height"].get<int>();
            config.spawnX = bz["spawnX"].get<int>();
            config.spawnY = bz["spawnY"].get<int>();
            config.respawnCooldownMs = bz.value("respawnCooldownMs", 300000.0f);
            bossZones.push_back(config);
        }
    }

    if (data.contains("dungeons")) {
        for (const auto& d: data["dungeons"]) {
            BossZoneConfig config;
            config.x = d["x"].get<int>();
            config.y = d["y"].get<int>();
            config.width = d["width"].get<int>();
            config.height = d["height"].get<int>();
            config.spawnX = config.x + config.width / 2;
            config.spawnY = config.y + config.height / 2;
            config.respawnCooldownMs = 300000.0f;
            bossZones.push_back(config);
        }
    }

    if (data.contains("forests")) {
        for (const auto& f: data["forests"]) {
            MapZoneRect rect{ZoneType::FOREST, f["x"].get<int>(), f["y"].get<int>(),
                             f["width"].get<int>(), f["height"].get<int>()};
            zoneRects.push_back(rect);
        }
    }

    if (data.contains("deserts")) {
        for (const auto& d: data["deserts"]) {
            MapZoneRect rect{ZoneType::DESERT, d["x"].get<int>(), d["y"].get<int>(),
                             d["width"].get<int>(), d["height"].get<int>()};
            zoneRects.push_back(rect);
        }
    }

    if (data.contains("safeZones")) {
        for (const auto& zone: data["safeZones"]) {
            safeZones.addZone(zone["name"].get<std::string>(), zone["x"].get<int>(),
                              zone["y"].get<int>(), zone["width"].get<int>(),
                              zone["height"].get<int>());
        }
    }

    if (data.contains("npcs")) {
        for (const auto& npc: data["npcs"]) {
            std::string typeStr = npc["type"].get<std::string>();
            NPCType type = NPCType::MERCHANT;  // Default fallback
            if (typeStr == "merchant")
                type = NPCType::MERCHANT;
            else if (typeStr == "banker")
                type = NPCType::BANKER;
            else if (typeStr == "priest")
                type = NPCType::PRIEST;

            Position pos{npc["x"].get<int>(), npc["y"].get<int>()};
            npcs.addNPC(type, pos);
        }
    }

    if (options.spawnMonsters && data.contains("monsters")) {
        for (const auto& monster: data["monsters"]) {
            std::string typeStr = monster["type"].get<std::string>();
            std::optional<NPCType> type;
            if (typeStr == "goblin")
                type = NPCType::GOBLIN;
            else if (typeStr == "skeleton")
                type = NPCType::SKELETON;
            else if (typeStr == "zombie")
                type = NPCType::ZOMBIE;
            else if (typeStr == "spider")
                type = NPCType::SPIDER;
            else if (typeStr == "orc")
                type = NPCType::ORC;
            else if (typeStr == "golem")
                type = NPCType::GOLEM;
            if (!type)
                continue;
            Position pos{monster["x"].get<int>(), monster["y"].get<int>()};
            monsterSpawns.push_back({*type, pos});
        }
    }

    if (data.contains("obstacles")) {
        for (const auto& obs: data["obstacles"]) {
            int x = obs["x"].get<int>();
            int y = obs["y"].get<int>();
            MapElement el;
            el.type = MapElementType::OBSTACLE;
            el.area = {x, y, 1, 1};
            mapElements.push_back(el);
        }
        generate_collision_grid();
    }

    if (options.spawnGroundItems && data.contains("items")) {
        for (const auto& item: data["items"]) {
            int id = item["id"].get<int>();
            int x = item["x"].get<int>();
            int y = item["y"].get<int>();
            int amount = item.value("amount", 1);
            placeItem(Position{x, y}, static_cast<uint32_t>(id), static_cast<uint16_t>(amount));
        }
    }

    return true;
}

const std::vector<MapMonsterSpawn>& Map::getMonsterSpawns() const { return monsterSpawns; }

int Map::heightLimit() const { return this->height; }

int Map::widthLimit() const { return this->width; }

/*Area Map::initArea(const int x, const int y, const int width, const int height) {
    Area area;
    area.x = x;
    area.y = this->height - y - height;
    area.width = width;
    area.height = height;
    return area;
}*/

void Map::generate_collision_grid() {
    // Marcamos en la matriz qué celdas específicas están ocupadas
    collisionLayer.clear();
    for (const auto& element: mapElements) {
        if (element.type == MapElementType::OBSTACLE) {
            // Si el obstáculo abarca varias celdas, las marcamos todas
            for (int x = element.area.x; x < element.area.x + element.area.width; ++x) {
                for (int y = element.area.y; y < element.area.y + element.area.height; ++y) {
                    collisionLayer.setSolid(x, y, true);
                }
            }
        }
    }
}

bool Map::hasLineOfSight(const Position& from, const Position& to) const {
    return collisionLayer.hasLineOfSight(from, to);
}

bool Map::playerColision(float pos_x, float pos_y) const {
    return collisionLayer.checkPlayerCollision(pos_x, pos_y);
}

bool Map::isCitizenArea(float pos_x, float pos_y) const {
    return safeZones.isSafeZone(pos_x, pos_y);
}

bool Map::isSafeZone(float pos_x, float pos_y) const { return safeZones.isSafeZone(pos_x, pos_y); }

bool Map::placeItem(const Position& pos, uint32_t itemId, uint16_t amount) {
    if (!pos.isWithinBounds(width, height))
        return false;
    return groundItems.placeItem(pos, itemId, amount);
}

std::optional<Position> Map::placeItemNearby(const Position& pos, uint32_t itemId,
                                             uint16_t amount) {
    if (pos.isWithinBounds(width, height) && !groundItems.hasItemAt(pos) &&
        !collisionLayer.isSolid(pos.x, pos.y)) {
        groundItems.placeItem(pos, itemId, amount);
        return pos;
    }

    // Búsqueda en espiral dinámica
    const int MAX_RADIUS = 50;
    for (int r = 1; r <= MAX_RADIUS; ++r) {
        for (int dx = -r; dx <= r; ++dx) {
            for (int dy = -r; dy <= r; ++dy) {
                // Chequear solo el contorno del cuadrado de radio r
                if (std::abs(dx) == r || std::abs(dy) == r) {
                    Position adj{pos.x + dx, pos.y + dy};
                    if (adj.isWithinBounds(width, height) &&
                        !collisionLayer.isSolid(adj.x, adj.y) && !groundItems.hasItemAt(adj)) {
                        groundItems.placeItem(adj, itemId, amount);
                        return adj;
                    }
                }
            }
        }
    }
    return std::nullopt;  // Todo el radio está ocupado
}

std::optional<GroundItem> Map::pickUpItem(const Position& pos) {
    return groundItems.pickUpItem(pos);
}

bool Map::hasItemAt(const Position& pos) const { return groundItems.hasItemAt(pos); }

std::vector<std::pair<Position, GroundItem>> Map::getGroundItemsSnapshot() const {
    const auto& items = groundItems.getAllItems();
    return std::vector<std::pair<Position, GroundItem>>(items.begin(), items.end());
}

std::vector<GroundItemPersistData> Map::getGroundItemsPersistData() const {
    return groundItems.toPersistData();
}

void Map::restoreGroundItems(const std::vector<GroundItemPersistData>& data) {
    groundItems.fromPersistData(data);
}

void Map::addSafeZone(const std::string& name, int x, int y, int w, int h) {
    safeZones.addZone(name, x, y, w, h);
}

std::string Map::getSafeZoneName(float pos_x, float pos_y) const {
    return safeZones.getZoneName(pos_x, pos_y);
}

uint32_t Map::addNPC(NPCType type, const Position& pos) { return npcs.addNPC(type, pos); }

std::optional<NPCSpawn> Map::findNPCNear(const Position& pos, int range) const {
    auto nearby = npcs.findNPCsInRange(pos, range);
    if (!nearby.empty()) {
        return nearby.front();
    }
    return std::nullopt;
}

const std::vector<NPCSpawn>& Map::getAllNPCs() const { return npcs.getAllNPCs(); }

const std::vector<MapElement>& Map::getElements() const { return this->mapElements; }

bool Map::canMoveTo(const Position& pos) const {
    if (!pos.isWithinBounds(this->width, this->height)) {
        return false;
    }
    // collisionLayer nos dice si hay obstáculo estático
    // entityCollisionLayer nos dice si hay una entidad ocupando
    return !collisionLayer.isSolid(pos.x, pos.y) && !entityCollisionLayer.isSolid(pos.x, pos.y);
}

std::optional<Position> Map::findClosestFreePosition(const Position& origin, int maxRadius) const {
    if (canMoveTo(origin)) {
        return origin;
    }

    for (int r = 1; r <= maxRadius; ++r) {
        for (int dx = -r; dx <= r; ++dx) {
            for (int dy = -r; dy <= r; ++dy) {
                // Chequear el contorno del cuadrado de radio r
                if (std::abs(dx) == r || std::abs(dy) == r) {
                    Position candidate{origin.x + dx, origin.y + dy};
                    if (canMoveTo(candidate)) {
                        return candidate;
                    }
                }
            }
        }
    }
    return std::nullopt;
}

void Map::setEntityCollision(int x, int y, bool isSolid) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        entityCollisionLayer.setSolid(x, y, isSolid);
    }
}

bool Map::isTileSolid(float x, float y) const {
    return collisionLayer.isSolid(static_cast<int>(x), static_cast<int>(y));
}
