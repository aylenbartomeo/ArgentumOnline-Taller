#include "GameLoop.h"

#include <iostream>
#include <variant>

#include "../include/model/ServerEvents.h"

GameLoop::GameLoop(Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor,
                   const std::filesystem::path& configPath, const std::string& persistenceDir):
        isRunning(true),
        gameQueue(gameQueue),
        monitor(monitor),
        itemRegistry(configPath),
        playerDataStore(persistenceDir),
        world(1, "Server", itemRegistry) {
    world.loadMap("maps/defaultMap.json");
}

void GameLoop::run() {
    const int MS_PER_FRAME = 33;

    try {
        while (isRunning) {
            auto start_time = std::chrono::steady_clock::now();

            processInputs();
            dispatchWorldEvents();

            updateWorld(MS_PER_FRAME);
            dispatchWorldEvents();

            broadcastState();

            timeSinceLastSave += MS_PER_FRAME / 1000.0f;
            if (timeSinceLastSave >= SAVE_INTERVAL_SECONDS) {
                persistOnlinePlayers();
                timeSinceLastSave = 0.0f;
            }

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
            std::cout << "[GAMELOOP] Player joined: " << joinData.username << std::endl;

            auto savedData = playerDataStore.loadPlayerData(joinData.username);
            world.addPlayer(joinData.clientId, joinData.username, savedData);

            // 2. Un jugador se desconecta
        } else if (std::holds_alternative<DisconnectEvent>(event)) {
            DisconnectEvent discData = std::get<DisconnectEvent>(event);
            std::cout << "[GAMELOOP] Player " << discData.clientId << " requested disconnect."
                      << std::endl;

            // Extraer y persistir TODA la data antes de borrar al jugador
            auto username = world.getPlayerUsername(discData.clientId);
            auto persistData = world.getPlayerPersistData(discData.clientId);

            if (username.has_value() && persistData.has_value()) {
                playerDataStore.savePlayerData(username.value(), persistData.value());
            }

            world.removePlayer(discData.clientId);

            // 3. Checkeo de comandos in-game
        } else if (std::holds_alternative<PlayerCommand>(event)) {
            PlayerCommand pCmd = std::get<PlayerCommand>(event);

            if (std::holds_alternative<StartMoveDTO>(pCmd.command)) {
                StartMoveDTO move_dto = std::get<StartMoveDTO>(pCmd.command);
                std::cout << "[GAMELOOP] Player " << pCmd.clientId
                          << " requested move to: " << static_cast<int>(move_dto.direction)
                          << std::endl;
                world.moveEntity(pCmd.clientId, move_dto.direction);

            } else if (std::holds_alternative<AttackDTO>(pCmd.command)) {
                AttackDTO attack_dto = std::get<AttackDTO>(pCmd.command);
                world.playerAttack(pCmd.clientId, attack_dto.targetId);

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

void GameLoop::dispatchWorldEvents() {
    auto events = world.pollEvents();
    for (const auto& ev: events) {
        monitor.sendToClient(ev.targetDbId, ChatDTO{ev.message});
    }
}

void GameLoop::broadcastState() {
    // Sacamos la foto del estado actual de tu partida y mapa
    SnapshotDTO snap = world.generateSnapshot();
    // El monitor lo distribuye concurrentemente
    monitor.broadcast(snap);
}

void GameLoop::persistOnlinePlayers() {
    auto dbIds = world.getOnlinePlayerDbIds();
    for (uint32_t dbId: dbIds) {
        auto username = world.getPlayerUsername(dbId);
        auto persistData = world.getPlayerPersistData(dbId);

        if (username.has_value() && persistData.has_value()) {
            playerDataStore.savePlayerData(username.value(), persistData.value());
        }
    }
}

void GameLoop::stop() { isRunning = false; }
