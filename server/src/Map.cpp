#include "Map.h"

#include <cmath>
#include <fstream>
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
}

void Map::setDimensions(int w, int h) {
    this->width = w;
    this->height = h;
    // Redimensionamos la matriz de colisiones para que coincida con el nuevo tamaño
    collisionLayer.resize(w, h);
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

    return true;
}

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

    // Buscar en las 8 posiciones adyacentes
    static const int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    static const int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};

    for (int i = 0; i < 8; ++i) {
        Position adj{pos.x + dx[i], pos.y + dy[i]};
        if (adj.isWithinBounds(width, height) && !collisionLayer.isSolid(adj.x, adj.y) &&
            !groundItems.hasItemAt(adj)) {
            groundItems.placeItem(adj, itemId, amount);
            return adj;
        }
    }
    return std::nullopt;  // No hay espacio libre
}

std::optional<GroundItem> Map::pickUpItem(const Position& pos) {
    return groundItems.pickUpItem(pos);
}

bool Map::hasItemAt(const Position& pos) const { return groundItems.hasItemAt(pos); }

std::vector<std::pair<Position, GroundItem>> Map::getGroundItemsSnapshot() const {
    const auto& items = groundItems.getAllItems();
    return std::vector<std::pair<Position, GroundItem>>(items.begin(), items.end());
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
    // collisionLayer nos dice si hay obstáculo
    return !collisionLayer.isSolid(pos.x, pos.y);
}
