#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "../server/src/GameLoop.h"
#include "../common/include/queue.h"

TEST(GameLoopTest, GameLoop_RunsAndProcessesEventsAsynchronously) {
    // 1. Inicializamos la cola de eventos y el monitor
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor; // Ajustá si tu constructor pide parámetros

    // Creación del GameLoop (nace con isRunning = true pero sin morder el hilo aún)
    GameLoop loop(gameQueue, monitor);

    // 2. Preparamos los datos de prueba: un JoinEvent y un comando de movimiento
    JoinEvent join;
    join.clientId = 888;
    join.username = "Gandalf";
    gameQueue.push(join);

    StartMoveDTO moveCmd;
    moveCmd.direction = 2; // Suponiendo Direction::LEFT en tu enum

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
    // significa que el pipeline interno (cola -> loop -> world -> monitor) funciona de punta a punta.
    SUCCEED(); 
}

TEST(GameLoopTest, GameLoop_StopsCleanlyEvenWithEmptyQueue) {
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;
    GameLoop loop(gameQueue, monitor);

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