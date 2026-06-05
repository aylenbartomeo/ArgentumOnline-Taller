#include "GameLoop.h"

#include <iostream>
#include <variant>
#include <vector>

#include "../include/ServerEvents.h"
#include "config/MonsterConfigLoader.h"
#include "dto/ClanCommandDTO.h"

GameLoop::GameLoop(Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor,
                   const std::filesystem::path& configDir, const WorldConfig& wConfig):
        isRunning(true),
        gameQueue(gameQueue),
        monitor(monitor),
        itemRegistry(configDir / "items.toml"),
        playerDataStore(wConfig.worldDir),
        characterConfigs(
                CharacterConfigLoader::loadCharacterConfigs(configDir / "characters.toml")),
        inventoryConfig(InventoryConfigLoader::loadInventoryConfig(configDir / "inventory.toml")),
        worldConfig(wConfig),
        worldDataStore("worlds/"),
        world(wConfig.worldId, wConfig.worldName, itemRegistry, characterConfigs, inventoryConfig) {
    if (worldConfig.isNewWorld) {
        world.loadMap(worldConfig.baseMapPath, true);
    } else {
        world.loadMap(worldConfig.baseMapPath, false);
        MonsterConfigs mConfigs;
        try {
            mConfigs = MonsterConfigLoader::loadMonsterConfigs("config/monsters.toml");
        } catch (...) {}
        world.restoreMonsters(worldDataStore.loadMonsters(worldConfig.worldId), mConfigs);
        world.restoreGroundItems(worldDataStore.loadGroundItems(worldConfig.worldId));

        auto [clanHeaders, clanMembers, clanPending, clanBanned] =
                worldDataStore.loadClans(worldConfig.worldId);
        world.restoreClans({clanHeaders, clanMembers, clanPending, clanBanned});

        auto [bankHeaders, bankSlots] = worldDataStore.loadBankAccounts(worldConfig.worldId);
        world.restoreBank({bankHeaders, bankSlots});
    }
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
                persistWorldState();
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
        persistWorldState();  // Guardado final al salir
    } catch (const std::exception& e) {
        std::cerr << "[GAMELOOP] Error: " << e.what() << std::endl;
        persistWorldState();
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
            std::cout << "[GAMELOOP] Jugador conectado: " << joinData.username << std::endl;

            auto savedData = playerDataStore.loadPlayerData(joinData.username);
            std::optional<Position> savedPos = std::nullopt;
            if (savedData.has_value()) {
                savedPos = Position{savedData->posX, savedData->posY};
            }
            world.addPlayer(joinData.clientId, joinData.username, savedData);

            // 2. Un jugador se desconecta
        } else if (std::holds_alternative<DisconnectEvent>(event)) {
            DisconnectEvent discData = std::get<DisconnectEvent>(event);
            std::cout << "[GAMELOOP] Jugador " << discData.clientId << " solicitó desconexión."
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
                std::cout << "[GAMELOOP] Jugador " << pCmd.clientId
                          << " solicito moverse a: " << static_cast<int>(move_dto.direction)
                          << std::endl;
                world.moveEntity(pCmd.clientId, move_dto.direction);

            } else if (std::holds_alternative<AttackDTO>(pCmd.command)) {
                AttackDTO attack_dto = std::get<AttackDTO>(pCmd.command);
                world.playerAttack(pCmd.clientId, attack_dto.targetId);

            } else if (std::holds_alternative<DropItemDTO>(pCmd.command)) {
                DropItemDTO drop_dto = std::get<DropItemDTO>(pCmd.command);
                world.dropItem(pCmd.clientId, drop_dto.slot, drop_dto.amount);
            } else if (std::holds_alternative<GrabItemDTO>(pCmd.command)) {
                world.pickUpItem(pCmd.clientId);
            } else if (std::holds_alternative<MeditateDTO>(pCmd.command)) {
                world.playerMeditate(pCmd.clientId);
            } else if (std::holds_alternative<ResurrectDTO>(pCmd.command)) {
                world.playerResurrect(pCmd.clientId);
            } else if (std::holds_alternative<SelectNpcDTO>(pCmd.command)) {
                SelectNpcDTO selectDto = std::get<SelectNpcDTO>(pCmd.command);
                std::cout << "[GAMELOOP] Jugador " << pCmd.clientId
                          << " hizo clic en el NPC: " << selectDto.npcId << std::endl;
                world.playerInteract(pCmd.clientId, selectDto.npcId);
            } else if (std::holds_alternative<NpcCommandDTO>(pCmd.command)) {
                NpcCommandDTO cmdDto = std::get<NpcCommandDTO>(pCmd.command);
                std::cout << "[GAMELOOP] Jugador " << pCmd.clientId
                          << " ejecuto comando de NPC tipo: " << static_cast<int>(cmdDto.type)
                          << std::endl;
                world.playerExecuteNpcCommand(pCmd.clientId, cmdDto);
            } else if (std::holds_alternative<ClanCommandDTO>(pCmd.command)) {
                ClanCommandDTO clanCmd = std::get<ClanCommandDTO>(pCmd.command);
                world.processClanCommand(pCmd.clientId, clanCmd);
            } else if (std::holds_alternative<ChatDTO>(pCmd.command)) {
                // Por ahora: broadcast a todos (chat general).
                const std::string& msg = std::get<ChatDTO>(pCmd.command).message;
                auto senderName = world.getPlayerUsername(pCmd.clientId);
                if (senderName.has_value()) {
                    ChatDTO broadcast;
                    broadcast.message = "[" + senderName.value() + "] " + msg;
                    monitor.broadcastChat(broadcast);
                }

            } else if (std::holds_alternative<PrivateChatDTO>(pCmd.command)) {
                const PrivateChatDTO& priv = std::get<PrivateChatDTO>(pCmd.command);
                auto senderName = world.getPlayerUsername(pCmd.clientId);
                if (!senderName.has_value())
                    return;

                // Resolver nick destinatario → dbId
                uint32_t recipientId = world.resolveNickToDbId(priv.recipientNick);

                if (recipientId == 0) {
                    // Destinatario no encontrado: avisar al remitente
                    ChatDTO err;
                    err.message = "[Server] Usuario '" + priv.recipientNick + "' no encontrado.";
                    monitor.sendToClient(pCmd.clientId, err);
                    continue;
                }

                // Enviar al destinatario
                ChatDTO toRecipient;
                toRecipient.message = "[PM de " + senderName.value() + "] " + priv.message;
                monitor.sendToClient(recipientId, toRecipient);

                // Confirmar al remitente
                ChatDTO toSender;
                toSender.message = "[PM → " + priv.recipientNick + "] " + priv.message;
                monitor.sendToClient(pCmd.clientId, toSender);
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
        if (ev.type == EventType::BROADCAST) {
            monitor.broadcastChat(ChatDTO{ev.message});
        } else {
            monitor.sendToClient(ev.targetDbId, ChatDTO{ev.message});
        }
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

void GameLoop::persistWorldState() {
    persistOnlinePlayers();
    worldDataStore.saveMonsters(worldConfig.worldId, world.getMonstersPersistData());
    worldDataStore.saveGroundItems(worldConfig.worldId, world.getGroundItemsPersistData());

    auto clanData = world.getClansPersistData();
    worldDataStore.saveClans(worldConfig.worldId, clanData.headers, clanData.members,
                             clanData.pending, clanData.banned);

    auto bankData = world.getBankPersistData();
    worldDataStore.saveBankAccounts(worldConfig.worldId, bankData.headers, bankData.slots);
}

void GameLoop::stop() { isRunning = false; }
