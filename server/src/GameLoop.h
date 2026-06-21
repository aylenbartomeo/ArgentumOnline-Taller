#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>

#include "../../common/include/dto/ClientCommands.h"
#include "../../common/include/queue.h"
#include "../../common/include/thread.h"
#include "config/CharacterConfig.h"
#include "config/CharacterConfigLoader.h"
#include "config/InventoryConfigLoader.h"
#include "dto/CommandDTO.h"
#include "dto/Snapshot.h"
#include "model/items/ItemRegistry.h"
#include "persistence/PlayerDataStore.h"
#include "persistence/WorldDataStore.h"
#include "server/src/config/ServerConfigLoader.h"

#include "ConnectionMonitor.h"
#include "World.h"

struct WorldConfig {
    uint32_t worldId;
    std::string worldName;
    std::string baseMapPath;
    std::string worldDir;
    bool isNewWorld;
};

class GameLoop: public Thread {
private:
    std::atomic<bool> isRunning;
    Queue<GameEvent>& gameQueue;
    ConnectionMonitor& monitor;
    ItemRegistry itemRegistry;
    PlayerDataStore playerDataStore;
    CharacterConfigs characterConfigs;
    const InventoryConfig inventoryConfig;
    WorldConfig worldConfig;
    WorldDataStore worldDataStore;
    World world;

    std::unordered_map<uint32_t, std::string> pendingCreations;
    // Mapa que asocia el tipo de DTO con su función ejecutora (Handler)
    std::unordered_map<std::type_index, std::function<void(uint32_t, const PlayerCommand&)>>
            commandDispatcher;
    // Timer para guardado periódico
    static constexpr float SAVE_INTERVAL_SECONDS = 30.0f;
    float timeSinceLastSave = 0.0f;

    void registerHandlers();  // Inicializa el mapa de comandos

    // Helper en plantilla para registrar comandos de forma fluida y auto-gestionada
    template <typename T>
    void registerCommand(std::function<void(uint32_t, const T&)> handler) {
        commandDispatcher[typeid(T)] = [handler](uint32_t clientId, const PlayerCommand& pCmd) {
            handler(clientId, std::get<T>(pCmd.command));
        };
    }

    void processInputs();
    void dispatchWorldEvents();
    void updateWorld(float delta_time);
    void broadcastState();
    void persistOnlinePlayers();
    void persistWorldState();

public:
    GameLoop(Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor,
             const std::filesystem::path& configDir, const WorldConfig& wConfig,
             const ServerConfig& serverConfig);

    void run() override;
    void stop() override;

    // Getter para pruebas unitarias
    World& getWorld() { return this->world; }

    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;
};

#endif
