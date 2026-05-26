#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

#include "../../common/include/dto/ClientCommands.h"
#include "../../common/include/queue.h"
#include "../../common/include/thread.h"
#include "dto/CommandDTO.h"
#include "dto/Snapshot.h"

#include "ConnectionMonitor.h"
#include "World.h"
#include "model/items/ItemRegistry.h"
#include "persistence/PlayerDataStore.h"

class GameLoop: public Thread {
private:
    std::atomic<bool> isRunning;
    Queue<GameEvent>& gameQueue;
    ConnectionMonitor& monitor;
    ItemRegistry itemRegistry;
    PlayerDataStore playerDataStore;
    World world;

    // Timer para guardado periódico
    static constexpr float SAVE_INTERVAL_SECONDS = 30.0f;
    float timeSinceLastSave = 0.0f;

    void processInputs();
    void dispatchWorldEvents();
    void updateWorld(float delta_time);
    void broadcastState();
    void persistOnlinePlayers();

public:
    GameLoop(Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor, 
             const std::filesystem::path& configPath,
             const std::string& persistenceDir = "game_data/");

    void run() override;
    void stop() override;

    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;
};

#endif
