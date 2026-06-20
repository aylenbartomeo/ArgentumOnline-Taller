#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "../common/include/queue.h"
#include "../server/src/GameLoop.h"
#include "model/entities/Player.h"
#include "model/items/Consumable.h"

#include "TestHelpers.h"  // <--- Incluimos tu nuevo archivo unificado de helpers

// =========================================================================
// TEST 1: EJECUCIÓN ASÍNCRONA Y PROCESAMIENTO DE EVENTOS
// =========================================================================
TEST(GameLoopTest, GameLoop_RunsAndProcessesEventsAsynchronously) {
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;

    WorldConfig wConfig{1, "Test", "maps/defaultMap.json", "game_data/", true};
    GameLoop loop(gameQueue, monitor, "../config", wConfig, TestUtils::getTestServerConfig());

    JoinEvent join;
    join.clientId = 888;
    join.username = "Gandalf";
    gameQueue.push(join);

    StartMoveDTO moveCmd;
    moveCmd.direction = Movement::LEFT;

    PlayerCommand pCmd;
    pCmd.clientId = 888;
    pCmd.command = moveCmd;
    gameQueue.push(pCmd);

    std::thread hiloGameLoop(&GameLoop::run, &loop);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    loop.stop();

    if (hiloGameLoop.joinable()) {
        hiloGameLoop.join();
    }

    SUCCEED();
}

// =========================================================================
// TEST 2: APAGADO LIMPIO CON COLA VACÍA
// =========================================================================
TEST(GameLoopTest, GameLoop_StopsCleanlyEvenWithEmptyQueue) {
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;
    WorldConfig wConfig{1, "Test", "maps/defaultMap.json", "game_data/", true};
    GameLoop loop(gameQueue, monitor, "../config", wConfig, TestUtils::getTestServerConfig());

    // Lanzamos con la cola vacía
    std::thread hiloGameLoop(&GameLoop::run, &loop);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Frenamos inmediatamente
    loop.stop();

    if (hiloGameLoop.joinable()) {
        hiloGameLoop.join();
    }

    SUCCEED();
}

// =========================================================================
// TEST 3: EXPIRACIÓN DE CONSUMIBLES EN EL GAMELOOP
// =========================================================================
TEST(GameLoopTest, GameLoop_ConsumablesStopAffectingStatsAfterDuration) {
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;
    WorldConfig wConfig{1, "Test", "maps/defaultMap.json", "game_data/", true};
    GameLoop loop(gameQueue, monitor, "../config", wConfig, TestUtils::getTestServerConfig());

    JoinEvent join;
    join.clientId = 999;
    join.username = "TestPlayer";
    gameQueue.push(join);

    CreateCharacterDTO createCmd;
    createCmd.race = 0;            // Humano
    createCmd.characterClass = 0;  // Guerrero
    PlayerCommand pCmd;
    pCmd.clientId = 999;
    pCmd.command = createCmd;
    gameQueue.push(pCmd);

    // 2. Iniciamos el GameLoop
    std::thread hiloGameLoop(&GameLoop::run, &loop);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    Player* player = loop.getWorld().getPlayerById(999);

    if (!player) {
        loop.stop();
        if (hiloGameLoop.joinable()) {
            hiloGameLoop.join();
        }
        FAIL() << "El jugador no fue encontrado en el mundo";
    }

    uint8_t baseStrength = player->getStrength();

    Consumable strElixir(5001, "Elixir Corto", 100, ConsumableType::BOOST_STR, 100, 5);
    bool used = strElixir.use(*player);
    ASSERT_TRUE(used);

    EXPECT_EQ(player->getStrength(), baseStrength + 5);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    EXPECT_EQ(player->getStrength(), baseStrength);

    loop.stop();
    if (hiloGameLoop.joinable()) {
        hiloGameLoop.join();
    }
}
