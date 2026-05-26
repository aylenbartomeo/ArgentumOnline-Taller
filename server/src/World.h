#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../include/model/ServerEvents.h"
#include "dto/CommandDTO.h"
#include "dto/Snapshot.h"
#include "model/entities/Monster.h"
#include "model/entities/Player.h"

#include "Map.h"
#include "queue.h"

class ItemRegistry;

struct WorldEvent {
    uint32_t targetDbId;
    std::string message;
};

class World {
private:
    int worldId;
    std::string creatorPlayerName;
    const ItemRegistry& itemRegistry;

    Map map;
    std::unordered_map<uint32_t, std::unique_ptr<Player>> players;
    std::unordered_map<uint32_t, std::unique_ptr<Monster>> monsters;

    uint32_t nextEntityId = 1;
    std::unordered_map<uint32_t, uint32_t> dbIdToEntityId;

    std::vector<WorldEvent> outgoingEvents;

    // Busca un Attackable por ID (busca en players y luego en monsters)
    Attackable* findAttackable(uint32_t id);

    // IA de monstruos: busca al Player más cercano dentro de un rango
    Player* findNearestPlayer(const Monster& monster, int range);

    // Procesa el ataque de un Monster a un Player
    void monsterAttack(const Monster& monster, Player& target);

public:
    explicit World(int worldId, const std::string& creatorPlayerName,
                   const ItemRegistry& itemRegistry);

    // Métodos lógicos: Entrar y salir del mundo virtual
    bool addPlayer(uint32_t playerId, std::string& username,
                   const std::optional<Position>& savedPosition = std::nullopt);
    bool removePlayer(uint32_t playerId);

    bool loadMap(const std::string& path);

    // Gestión de monstruos
    uint32_t addMonster(NPCType type, Position pos, const MonsterConfig& config);

    /* Metodos de acciones de los personajes en el mundo */
    void moveEntity(uint32_t playerId, Movement movement);

    // Ataque genérico: el atacante (Player) busca al target en players Y monsters
    void playerAttack(uint32_t attackerId, uint32_t targetId);

    /* actualización del estado del mundo */
    std::vector<WorldEvent> pollEvents();

    void update(float delta_time);

    // Generación del estado actual para ser enviado por red
    SnapshotDTO generateSnapshot() const;

    // Getters para persistencia
    std::optional<Position> getPlayerPosition(uint32_t dbId) const;
    std::optional<std::string> getPlayerUsername(uint32_t dbId) const;
    std::vector<uint32_t> getOnlinePlayerDbIds() const;

    /* Getters y setters */
    std::string getCreatorPlayerName() const;
    int getWorldId() const;
    int getPlayerCount() const;
    bool isEmpty() const;  // Para que el servidor sepa cuándo destruir la partida

    // Retorna la posición de spawn inicial del mapa
    std::pair<float, float> getInitialPosition();

    // Para testing: permite colocar obstáculos en el mapa
    void setObstacleAt(int x, int y);

    ~World() = default;
};

#endif
