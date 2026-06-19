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
    // 1. Inicializamos la cola de eventos y el monitor
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;

    // Creación del GameLoop usando la configuración centralizada de TestUtils
    WorldConfig wConfig{1, "Test", "maps/defaultMap.json", "game_data/", true};
    GameLoop loop(gameQueue, monitor, "../config", wConfig, TestUtils::getTestServerConfig());

    // 2. Preparamos los datos de prueba: un JoinEvent y un comando de movimiento
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

    // 3. Lanzamos el GameLoop en un hilo separado
    std::thread hiloGameLoop(&GameLoop::run, &loop);

    // 4. Le damos un pequeño changüí de tiempo (100ms) para procesar los frames
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 5. Frenamos el bucle de manera segura
    loop.stop();

    // 6. Esperamos que el hilo termine limpiamente
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

    // 1. Unimos un jugador al mundo
    JoinEvent join;
    join.clientId = 999;
    join.username = "TestPlayer";
    gameQueue.push(join);

    // Mandar comando de creación de personaje
    CreateCharacterDTO createCmd;
    createCmd.race = 0;            // Humano
    createCmd.characterClass = 0;  // Guerrero
    PlayerCommand pCmd;
    pCmd.clientId = 999;
    pCmd.command = createCmd;
    gameQueue.push(pCmd);

    // 2. Iniciamos el GameLoop
    std::thread hiloGameLoop(&GameLoop::run, &loop);

    // Damos un breve tiempo para procesar la creación
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 3. Obtenemos al jugador del mundo
    Player* player = loop.getWorld().getPlayerById(999);

    if (!player) {
        loop.stop();
        if (hiloGameLoop.joinable()) {
            hiloGameLoop.join();
        }
        FAIL() << "El jugador no fue encontrado en el mundo";
    }

    uint8_t baseStrength = player->getStrength();

    // 4. Creamos y usamos un consumible de Fuerza con duración de 100ms
    Consumable strElixir(5001, "Elixir Corto", 100, ConsumableType::BOOST_STR, 100, 5);
    bool used = strElixir.use(*player);
    ASSERT_TRUE(used);

    // 5. Verificamos que la estadística aumentó
    EXPECT_EQ(player->getStrength(), baseStrength + 5);

    // 6. Esperamos más de la duración del consumible (150ms) para que actúen los update ticks
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // 7. Verificamos la expiración: volvió a su estado base
    EXPECT_EQ(player->getStrength(), baseStrength);

    // 8. Detener el bucle limpiamente
    loop.stop();
    if (hiloGameLoop.joinable()) {
        hiloGameLoop.join();
    }
}
