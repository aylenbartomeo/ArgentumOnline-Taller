#include "GameLoop.h"
#include <variant>

GameLoop::GameLoop(Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor):
        isRunning(true), gameQueue(gameQueue), monitor(monitor), world(1, "Server") {}

void GameLoop::run() {
    const int MS_PER_FRAME = 33;

    try {
        while (isRunning) {
            auto start_time = std::chrono::steady_clock::now();

            processInputs();

            updateWorld(MS_PER_FRAME);

            broadcastState();

            auto end_time = std::chrono::steady_clock::now();
            auto elapsed =
                    std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time)
                            .count();

            if (elapsed < MS_PER_FRAME) {
                std::this_thread::sleep_for(std::chrono::milliseconds(MS_PER_FRAME - elapsed));
            }
        }
    } catch (const std::exception& e) {
        isRunning = false;
    }
}

void GameLoop::processInputs() {
    GameEvent event;

    // Tu try_pop real de tu clase Queue devuelve bool. Desencolamos de forma segura.
    while (gameQueue.try_pop(event)) {
        
        // 1. CHEQUEO DE JOIN_EVENT
        if (std::holds_alternative<JoinEvent>(event)) {
            // std::get extrae el tipo exacto de la variante
            JoinEvent joinData = std::get<JoinEvent>(event);
            world.addPlayer(joinData.playerId, joinData.username);
        } 
        
        // 2. CHEQUEO DE COMMAND_DTO
        else if (std::holds_alternative<CommandDTO>(event)) {
            CommandDTO comando = std::get<CommandDTO>(event);
            switch (comando.type) {
                case ActionType::MOVE:
                    world.moveEntity(comando.playerId, comando.movement);
                    std::cout << "[GAMELOOP] Jugador " << comando.playerId << " se mueve en dirección: " << comando.movement << std::endl;
                    break;

                case ActionType::ATTACK:
                    world.playerAttack(comando.playerId);
                    std::cout << "[GAMELOOP] Jugador " << comando.playerId << " ejecutó un ataque." << std::endl;
                    break;

                case ActionType::DISCONNECT:
                    world.removePlayer(comando.playerId);
                    std::cout << "[GAMELOOP] Jugador " << comando.playerId << " solicita desconexión." << std::endl;
                    break;

                default:
                    break;
            }
        }
    }
}

void GameLoop::updateWorld(float delta_time) {
    // Le pasamos el tiempo transcurrido a tu partida para físicas o efectos temporales
    world.update(delta_time);
}

void GameLoop::broadcastState() {
    // Sacamos la foto del estado actual de tu partida y mapa
    SnapshotDTO snap = world.generateSnapshot();
    // El monitor lo distribuye concurrentemente
    monitor.broadcast(snap);
}

void GameLoop::stop() { isRunning = false; }
