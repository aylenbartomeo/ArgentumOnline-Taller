#include "GameLoop.h"
#include <variant>
#include "../include/model/ServerEvents.h"
#include <iostream>

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
            JoinEvent joinData = std::get<JoinEvent>(event);
            std::cout << "[GAMELOOP] Nace el jugador: " << joinData.username << std::endl;
            world.addPlayer(joinData.clientId, joinData.username);
            
        // 2. Un jugador se desconecta
        } else if (std::holds_alternative<DisconnectEvent>(event)) {
            DisconnectEvent discData = std::get<DisconnectEvent>(event);
            std::cout << "[GAMELOOP] Jugador " << discData.clientId << " solicita desconexión." << std::endl;
            world.removePlayer(discData.clientId);

        // 3. Checkeo de comandos in-game
        } else if (std::holds_alternative<PlayerCommand>(event)) {
            PlayerCommand pCmd = std::get<PlayerCommand>(event);

            if (std::holds_alternative<StartMoveDTO>(pCmd.command)) {
                StartMoveDTO move_dto = std::get<StartMoveDTO>(pCmd.command);
                std::cout << "[GAMELOOP] Jugador " << pCmd.clientId << " solicita moverse hacia: " << static_cast<int>(move_dto.direction) << std::endl;
                world.moveEntity(pCmd.clientId, move_dto.direction);

            } else if (std::holds_alternative<AttackDTO>(pCmd.command)) {
                // world.player_attack(pCmd.clientId);

            } else if (std::holds_alternative<DropItemDTO>(pCmd.command)) {
                // DropItemDTO drop_dto = std::get<DropItemDTO>(pCmd.command);
                // world.drop_item(pCmd.clientId, drop_dto.slot, drop_dto.amount);
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
