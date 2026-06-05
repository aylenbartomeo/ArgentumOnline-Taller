#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

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

    // Timer para guardado periódico
    static constexpr float SAVE_INTERVAL_SECONDS = 30.0f;
    float timeSinceLastSave = 0.0f;

    void processInputs();
    void dispatchWorldEvents();
    void updateWorld(float delta_time);
    void broadcastState();
    void persistOnlinePlayers();
    void persistWorldState();

public:
    GameLoop(Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor,
             const std::filesystem::path& configDir, const WorldConfig& wConfig);

    void run() override;
    void stop() override;

    // Getter para pruebas unitarias
    World& getWorld() { return this->world; }

    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;
};

#endif
