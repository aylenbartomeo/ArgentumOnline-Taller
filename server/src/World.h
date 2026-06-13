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
#include "dto/PlayerStatsDTO.h"
#include "dto/Snapshot.h"
#include "model/clan/ClanController.h"
#include "model/clan/ClanRepository.h"
#include "model/clan/ClanService.h"
#include "model/entities/EntityManager.h"
#include "model/entities/GlobalBank.h"
#include "model/entities/Monster.h"
#include "model/entities/Player.h"
#include "model/events/EventPublisher.h"
#include "model/items/ItemRegistry.h"
#include "model/systems/CombatSystem.h"
#include "model/systems/InteractionService.h"
#include "model/systems/ProjectileSystem.h"
#include "model/systems/ResurrectionService.h"
#include "model/systems/SpawnSystem.h"
#include "persistence/PlayerDataStore.h"
#include "persistence/WorldPersistData.h"

#include "Map.h"
#include "queue.h"

class World: public IWorldContext, public ICombatEventCallback {
private:
    int worldId;
    std::string creatorPlayerName;
    const ItemRegistry& itemRegistry;
    const InventoryConfig inventoryConfig;
    GlobalBank globalBank;

    Map map;
    EntityManager entityManager;

    // IDs de monstruos que murieron en este tick
    std::vector<uint32_t> deadMonsterIds;


    EventPublisher eventPublisher;

    ClanRepository clanRepo;
    ClanService clanService;
    ClanController clanController;
    bool enforceFairPlay = true;  // Regla de mundo: Modo arena

    CharacterConfigs characterConfigs;
    SpawnSystem spawnSystem;
    CombatSystem combatSystem;
    ProjectileSystem projectileSystem;
    ResurrectionService resurrectionService;
    InteractionService interactionService;

    // IA de monstruos: busca al Player más cercano dentro de un rango
    Player* findNearestPlayer(const Monster& monster, int range);

    // Mueve al monstruo hacia el objetivo (si está en cooldown no hace nada)
    void moveMonsterTowards(Monster& monster, const Position& targetPos);
    void monsterAttack(const Monster& monster, Player& target);

public:
    explicit World(int worldId, const std::string& creatorPlayerName,
                   const ItemRegistry& itemRegistry, const CharacterConfigs& configs,
                   const InventoryConfig& inventoryConfig);

    // Metodos de cheat para testing
    void playerCheat(uint32_t dbId, CheatType type);

    // Métodos lógicos: Entrar y salir del mundo virtual
    bool addPlayer(uint32_t dbId, std::string& username, Race race, CharacterClass cls,
                   const std::optional<PlayerPersistData>& savedData = std::nullopt);

    // Extrae la data completa de un jugador para persistencia
    std::optional<PlayerPersistData> getPlayerPersistData(uint32_t dbId) const;

    bool removePlayer(uint32_t playerId);

    bool loadMap(const std::string& path, bool spawnMonstersAndItems = true);

    // Gestión de monstruos y NPCs
    uint32_t addMonster(NPCType type, Position pos, const MonsterConfig& config);
    void spawnNPCs();

    /* Metodos de acciones de los personajes en el mundo */
    void moveEntity(uint32_t playerId, Movement movement);

    // Ataque genérico: el atacante (Player) busca al target en players Y monsters
    void playerAttack(uint32_t attackerId, uint32_t targetDbId);

    void playerShoot(uint32_t shooterDbId, float targetX, float targetY);

    // El método que busca en 'cityNpcs' cuando el cliente manda un click de interacción
    void playerInteract(uint32_t dbId, uint32_t targetId);
    void playerExecuteNpcCommand(uint32_t dbId, const NpcCommandDTO& dto);

    /* actualización del estado del mundo */
    std::vector<WorldEvent> pollEvents();

    void update(float delta_time);

    // Generación del estado actual para ser enviado por red
    SnapshotDTO generateSnapshot() const;
    std::optional<PlayerStatsDTO> getPlayerStatsDTO(uint32_t dbId) const;

    // Persistencia del mundo
    std::vector<MonsterPersistData> getMonstersPersistData() const;
    std::vector<GroundItemPersistData> getGroundItemsPersistData() const;
    void restoreMonsters(const std::vector<MonsterPersistData>& data,
                         const MonsterConfigs& configs);
    void restoreGroundItems(const std::vector<GroundItemPersistData>& data);

    // Persistencia de clanes
    ClanRepositoryPersistData getClansPersistData() const;
    void restoreClans(const ClanRepositoryPersistData& data);

    // Persistencia del banco global
    BankPersistData getBankPersistData() const;
    void restoreBank(const BankPersistData& data);

    // Getters para persistencia
    std::optional<Position> getPlayerPosition(uint32_t dbId) const;
    // CombatEventCallback
    void onMonsterDeath(const Monster& monster, uint32_t killerDbId) override;
    void onPlayerDeath(uint32_t dbId) override;

    // Helpers
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
    void equipItem(uint32_t dbId, uint8_t slot);
    void useItem(uint32_t dbId, uint8_t slot);
    void handlePlayerDeath(uint32_t dbId);
    void playerResurrect(uint32_t dbId);

    // Zonas seguras (delega al map)
    bool isSafeZone(float x, float y) const;

    // Metodos de IWorldContext
    uint16_t getPlayerLevel(uint32_t dbId) const override;
    uint32_t resolveNickToDbId(const std::string& nick) const override;
    std::optional<std::string> getPlayerUsername(uint32_t dbId) const override;

    // Procesa cualquier comando de clan enviado por un jugador.
    void processClanCommand(uint32_t senderDbId, const ClanCommandDTO& cmd);
    bool areClanmates(uint32_t a, uint32_t b) const;


    // Configuraciones de reglas de juego (para testing y ajustes de balance)
    void setFairPlayRules(bool enforce) {
        enforceFairPlay = enforce;
        combatSystem.setFairPlayRules(enforce);
    }
    void setClanMinLevel(uint16_t level) { clanService.setMinLevelToFound(level); }

    // Obtener una referencia al Player según su ID de base de datos
    Player* getPlayerById(uint32_t dbId);

    Interactable* resolveNpcTarget(uint32_t targetId, const Player& player) const;

    void publishInteractionResult(uint32_t dbId, const InteractionResult& res);


    ~World() override = default;
};

#endif
