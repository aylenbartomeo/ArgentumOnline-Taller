#ifndef MAP_H
#define MAP_H

#include <string>
#include <utility>
#include <vector>

#include "model/map/BossZone.h"
#include "model/map/CollisionLayer.h"
#include "model/map/GroundItemLayer.h"
#include "model/map/NPCLayer.h"
#include "model/map/SafeZoneLayer.h"

#include "position.h"

typedef enum { BACK, OBSTACLE, FLOOR, WEAPON } MapElementType;

struct Area {
    int x;
    int y;
    int width;
    int height;
};

// CitizenArea has been replaced by SafeZoneLayer

struct MapElement {
    Area area;
    MapElementType type;
};

struct MapMonsterSpawn {
    NPCType type;
    Position pos;
};

class Map {
private:
    int width, height;
    CollisionLayer collisionLayer;
    CollisionLayer entityCollisionLayer;
    std::vector<MapElement> mapElements;
    GroundItemLayer groundItems;
    SafeZoneLayer safeZones;
    NPCLayer npcs;
    std::vector<MapMonsterSpawn> monsterSpawns;
    std::vector<BossZoneConfig> bossZones;
    std::pair<float, float> spawn_point;

    //  Inicializa la matriz de colisiones en base a los mapElements cargados
    void generate_collision_grid();
    void loadFromToml(const std::string& filepath);

    Area initArea(const int x, const int y, const int width, const int height);

public:
    // Es mejor pasarle la ruta del mapa a cargar desde el inicio
    explicit Map(const std::string& toml_filepath);
    Map();  // Para pruebas con mapa vacío

    /* Devuelven las dimensiones del mapa */
    int heightLimit() const;
    int widthLimit() const;

    void setEntityCollision(int x, int y, bool isSolid);

    /* Retorna la posición inicial segura para un jugador */
    std::pair<float, float> getInitialPosition();

    struct MapLoadOptions {
        bool spawnMonsters = true;
        bool spawnGroundItems = true;
    };
    bool loadSpawnFromJson(const std::string& path);
    bool loadSpawnFromJson(const std::string& path, const MapLoadOptions& options);

    /* Retorna true si la posicion es de alguna zona segura, false en caso contrario */
    bool isCitizenArea(float pos_x, float pos_y) const;
    bool isSafeZone(float pos_x, float pos_y) const;

    /* Items en el suelo */
    bool placeItem(const Position& pos, uint32_t itemId, uint16_t amount);
    std::optional<Position> placeItemNearby(const Position& pos, uint32_t itemId, uint16_t amount);
    std::optional<GroundItem> pickUpItem(const Position& pos);
    bool hasItemAt(const Position& pos) const;
    std::vector<std::pair<Position, GroundItem>> getGroundItemsSnapshot() const;
    std::vector<GroundItemPersistData> getGroundItemsPersistData() const;
    void restoreGroundItems(const std::vector<GroundItemPersistData>& data);

    /* Zonas seguras */
    void addSafeZone(const std::string& name, int x, int y, int w, int h);
    std::string getSafeZoneName(float pos_x, float pos_y) const;

    /* NPCs ciudadanos */
    uint32_t addNPC(NPCType type, const Position& pos);
    std::optional<NPCSpawn> findNPCNear(const Position& pos, int range) const;
    const std::vector<NPCSpawn>& getAllNPCs() const;

    /* Monstruos cargados desde el mapa */
    const std::vector<MapMonsterSpawn>& getMonsterSpawns() const;
    const std::vector<BossZoneConfig>& getBossZones() const { return bossZones; }

    /* Expone los elementos para que el GameLoop arme los snapshots compartidos */
    const std::vector<MapElement>& getElements() const;

    /* Retorna true si hay línea de visión sin obstáculos */
    bool hasLineOfSight(const Position& from, const Position& to) const;
    bool playerColision(float pos_x, float pos_y) const;

    /* Metodos para setear los atributos del mapa en testing */
    void setDimensions(int w, int h);
    void setCitizenArea(int x, int y, int w, int h);
    void setSpawnPoint(float x, float y);
    void setObstacleInGrid(int cell_x, int cell_y, bool is_solid);

    // Retorna true si la posición es válida para moverse:
    // está dentro de los límites del mapa Y no hay obstáculo en collision_grid.
    bool canMoveTo(const Position& pos) const;

    // Verifica la colision para proyectiles
    bool isTileSolid(float x, float y) const;

    std::optional<Position> findClosestFreePosition(const Position& origin, int maxRadius) const;
};

#endif
