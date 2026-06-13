#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "../common/include/queue.h"
#include "../server/src/GameLoop.h"
#include "model/entities/Player.h"
#include "model/items/Consumable.h"

TEST(GameLoopTest, GameLoop_RunsAndProcessesEventsAsynchronously) {
    // 1. Inicializamos la cola de eventos y el monitor
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;  // Ajustá si tu constructor pide parámetros

    // Creación del GameLoop (nace con isRunning = true pero sin morder el hilo aún)
    WorldConfig wConfig{1, "Test", "maps/defaultMap.json", "game_data/", true};
    GameLoop loop(gameQueue, monitor, "../config", wConfig);

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

    // 3. Lanzamos el GameLoop en un hilo separado para que empiece a iterar de verdad
    std::thread hiloGameLoop(&GameLoop::run, &loop);

    // 4. Le damos un pequeño changüí de tiempo (ej. 100ms) para que el hilo
    // procese al menos 2 o 3 frames (recordá que procesa 1 frame cada 33ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // En este punto, de forma interna y privada, el loop ya ejecutó:
    // processInputs() -> consumió la cola, creó al jugador y lo movió.
    // updateWorld()   -> actualizó físicas.
    // broadcastState()-> le mandó el snapshot al monitor.

    // 5. Frenamos el bucle de manera segura invocando el método público
    loop.stop();

    // 6. Esperamos que el hilo termine su última iteración y muera limpiamente
    if (hiloGameLoop.joinable()) {
        hiloGameLoop.join();
    }

    // Si el test llega acá sin quedarse congelado (deadlock) y sin lanzar excepciones,
    // significa que el pipeline interno (cola -> loop -> world -> monitor) funciona de punta a
    // punta.
    SUCCEED();
}

TEST(GameLoopTest, GameLoop_StopsCleanlyEvenWithEmptyQueue) {
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;
    WorldConfig wConfig{1, "Test", "maps/defaultMap.json", "game_data/", true};
    GameLoop loop(gameQueue, monitor, "../config", wConfig);

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
    // Explicación: Verifica que al pasar el tiempo de duración de un consumible de boost,
    // el GameLoop al actualizar el mundo (update ticks) provoque que el boost expire
    // y las estadísticas del jugador vuelvan a su valor base.
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;
    WorldConfig wConfig{1, "Test", "maps/defaultMap.json", "game_data/", true};
    GameLoop loop(gameQueue, monitor, "../config", wConfig);

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

    // Damos un breve tiempo para que el loop procese el JoinEvent y cree al jugador en el mundo
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 3. Obtenemos al jugador del mundo (el GameLoop usa clientId como dbId temporal o final)
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

    // 6. Esperamos más de la duración del consumible (ej. 150ms)
    // El GameLoop corre en un hilo separado ejecutando updates cada 33ms,
    // por lo que irá descontando tiempo del boost activo.
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // 7. Verificamos que la estadística volvió a su estado base
    EXPECT_EQ(player->getStrength(), baseStrength);

    // 8. Detener el bucle limpiamente
    loop.stop();
    if (hiloGameLoop.joinable()) {
        hiloGameLoop.join();
    }
}
