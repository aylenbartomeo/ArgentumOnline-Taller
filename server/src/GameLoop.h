#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <string>
#include <atomic>
#include <chrono>
#include <thread>

#include "../../common/include/queue.h"
#include "../../common/include/thread.h"
#include "../../common/src/CommandDTO.h"
#include "../../common/src/Snapshot.h"

#include "ConnectionMonitor.h"
#include "World.h"

class GameLoop: public Thread {
private:
    std::atomic<bool> isRunning;
    Queue<GameEvent>& gameQueue;
    ConnectionMonitor& monitor;
    World world;

    void processInputs();
    void updateWorld(float delta_time);
    void broadcastState();

public:
    GameLoop(Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor);

    void run() override;
    void stop() override;

    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;
};

#endif
