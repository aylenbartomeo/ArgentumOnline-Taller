#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../include/ServerEvents.h"
#include "config/CharacterConfig.h"
#include "dto/ClientCommands.h"
#include "dto/CommandDTO.h"
#include "dto/Snapshot.h"
#include "model/clan/ClanController.h"
#include "model/clan/ClanRepository.h"
#include "model/clan/ClanService.h"
#include "model/entities/GlobalBank.h"
#include "model/entities/Monster.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"
#include "persistence/PlayerDataStore.h"
#include "persistence/WorldPersistData.h"

#include "Map.h"
#include "queue.h"

// Eventos de mundo (mensajes destinados al cliente); definidos aquí para evitar múltiples
// definiciones
struct WorldEvent {
    uint32_t targetDbId;
    std::string message;
};

class World: public IWorldContext {
private:
    int worldId;
    std::string creatorPlayerName;
    const ItemRegistry& itemRegistry;
    const InventoryConfig inventoryConfig;
    GlobalBank globalBank;

    Map map;
    std::unordered_map<uint32_t, std::unique_ptr<Player>> players;
    std::unordered_map<uint32_t, std::unique_ptr<Monster>> monsters;
    std::unordered_map<uint32_t, std::unique_ptr<Interactable>> cityNPCs;
    std::unordered_map<uint32_t, Interactable*> activeInteractions;

    uint32_t nextEntityId = 1;
    std::unordered_map<uint32_t, uint32_t> dbIdToEntityId;

    std::vector<WorldEvent> outgoingEvents;

    ClanRepository clanRepo;
    ClanService clanService;
    ClanController clanController;
    bool enforceFairPlay = true;  // Regla de mundo: Modo arena

    CharacterConfigs characterConfigs;

    // Busca un Attackable por ID (busca en players y luego en monsters)
    Attackable* findAttackable(uint32_t id);
    // Busca un interactable por ID
    Interactable* findInteractable(uint32_t id);

    // IA de monstruos: busca al Player más cercano dentro de un rango
    Player* findNearestPlayer(const Monster& monster, int range);

    // Mueve al monstruo hacia el objetivo (si está en cooldown no hace nada)
    void moveMonsterTowards(Monster& monster, const Player& target);

    // Procesa el ataque de un Monster a un Player
    void monsterAttack(const Monster& monster, Player& target);

public:
    explicit World(int worldId, const std::string& creatorPlayerName,
                   const ItemRegistry& itemRegistry, const CharacterConfigs& configs,
                   const InventoryConfig& inventoryConfig);

    // Métodos lógicos: Entrar y salir del mundo virtual
    bool addPlayer(uint32_t playerId, std::string& username,
                   const std::optional<PlayerPersistData>& savedData = std::nullopt);

    // Extrae la data completa de un jugador para persistencia
    std::optional<PlayerPersistData> getPlayerPersistData(uint32_t dbId) const;

    bool removePlayer(uint32_t playerId);

    bool loadMap(const std::string& path, bool spawnMonstersAndItems = true);

    // Gestión de monstruos y NPCs
    uint32_t addMonster(NPCType type, Position pos, const MonsterConfig& config);
    void spawnNPCs();
    void spawnMonsters();

    /* Metodos de acciones de los personajes en el mundo */
    void moveEntity(uint32_t playerId, Movement movement);

    // Ataque genérico: el atacante (Player) busca al target en players Y monsters
    void playerAttack(uint32_t attackerId, uint32_t targetDbId);

    // El método que busca en 'cityNpcs' cuando el cliente manda un click de interacción
    void playerInteract(uint32_t dbId, uint32_t targetId);
    void playerExecuteNpcCommand(uint32_t dbId, const NpcCommandDTO& dto);

    /* actualización del estado del mundo */
    std::vector<WorldEvent> pollEvents();

    void update(float delta_time);

    // Generación del estado actual para ser enviado por red
    SnapshotDTO generateSnapshot() const;

    // Persistencia del mundo
    std::vector<MonsterPersistData> getMonstersPersistData() const;
    std::vector<GroundItemPersistData> getGroundItemsPersistData() const;
    void restoreMonsters(const std::vector<MonsterPersistData>& data,
                         const MonsterConfigs& configs);
    void restoreGroundItems(const std::vector<GroundItemPersistData>& data);

    // Persistencia de clanes
    void getClansPersistData(std::vector<ClanHeaderPersistData>& headers,
                             std::vector<std::vector<ClanPlayerPersistData>>& members,
                             std::vector<std::vector<ClanPlayerPersistData>>& pending,
                             std::vector<std::vector<ClanPlayerPersistData>>& banned) const;
    void restoreClans(const std::vector<ClanHeaderPersistData>& headers,
                      const std::vector<std::vector<ClanPlayerPersistData>>& members,
                      const std::vector<std::vector<ClanPlayerPersistData>>& pending,
                      const std::vector<std::vector<ClanPlayerPersistData>>& banned);

    // Persistencia del banco global
    void getBankPersistData(std::vector<BankAccountHeaderPersistData>& headers,
                            std::vector<std::vector<BankSlotPersistData>>& slots) const;
    void restoreBank(const std::vector<BankAccountHeaderPersistData>& headers,
                     const std::vector<std::vector<BankSlotPersistData>>& slots);

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

    // Items en el suelo (delega al map)
    bool placeItemOnGround(const Position& pos, uint32_t itemId, uint16_t amount);
    std::optional<Position> placeItemNearby(const Position& pos, uint32_t itemId, uint16_t amount);
    std::optional<GroundItem> pickUpItemFromGround(const Position& pos);

    // Acciones de comandos del jugador
    void pickUpItem(uint32_t dbId);
    void playerMeditate(uint32_t dbId);
    void dropItem(uint32_t dbId, uint8_t slot, uint16_t amount);
    void handlePlayerDeath(uint32_t dbId);
    void playerResurrect(uint32_t dbId);

    struct PendingResurrection {
        uint32_t playerDbId;
        float remainingTimeMs;
        Position targetPos;
    };
    std::vector<PendingResurrection> pendingResurrections;

    // Zonas seguras (delega al map)
    bool isSafeZone(float x, float y) const;

    uint16_t getPlayerLevel(uint32_t dbId) const override;
    uint32_t resolveNickToDbId(const std::string& nick) const override;

    // Procesa cualquier comando de clan enviado por un jugador.
    void processClanCommand(uint32_t senderDbId, const ClanCommandDTO& cmd);

    // Devuelve cuántos clanmates del atacante están cerca de su posición
    int countNearbyClanmates(uint32_t playerDbId, int range) const;

    // Verdadero si ambos jugadores pertenecen al mismo clan
    bool areClanmates(uint32_t playerADbId, uint32_t playerBDbId) const;

    // Configuraciones de reglas de juego (para testing y ajustes de balance)
    void setFairPlayRules(bool enforce) { enforceFairPlay = enforce; }
    void setClanMinLevel(uint16_t level) { clanService.setMinLevelToFound(level); }

    // Obtener una referencia al Player según su ID de base de datos
    Player* getPlayerById(uint32_t dbId);

    ~World() = default;
};

#endif
