#include "GameLoop.h"

GameLoop::GameLoop(Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor):
        isRunning(true), gameQueue(gameQueue), monitor(monitor) {}

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

    while (gameQueue.try_pop(event)) {
        if (std::holds_alternative<JoinEvent>(event)) {
            JoinEvent joinData = std::get<JoinEvent>(event);

            // world.add_player(joinData.clientId, joinData.username);
            std::cout << "[GAMELOOP] Nace el jugador: " << joinData.username << std::endl;
        } else if (std::holds_alternative<CommandDTO>(event)) {
            CommandDTO comando = std::get<CommandDTO>(event);

            switch (comando.type) {
                case ActionType::MOVE:
                    // world.move_entity(comando.clientId, comando.movement);
                    break;

                case ActionType::ATTACK:
                    // world.player_attack(comando.clientId);
                    break;

                case ActionType::DISCONNECT:
                    // world.remove_player(comando.clientId);
                    break;

                default:
                    break;
            }
        }
    }
}

void GameLoop::updateWorld(float delta_time) {
    // Acá se apdatea todo..
    (void)delta_time;
    // world.update_monsters(delta_time);
    // world.update_spawns(delta_time);
}

void GameLoop::broadcastState() {
    // 1. Le pedimos al mundo la foto actual
    // SnapshotDTO snap = world.get_snapshot();
    SnapshotDTO snap;  // (Dummy temporal para que compile)

    // 2. Le pasamos el paquete al Monitor para que lo distribuya a las colas de los clientes
    monitor.broadcast(snap);
}

void GameLoop::stop() { isRunning = false; }
