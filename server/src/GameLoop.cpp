#include "GameLoop.h"

#include <iostream>
#include <variant>
#include <vector>

#include "../include/ServerEvents.h"
#include "config/MonsterConfigLoader.h"
#include "dto/ClanCommandDTO.h"
#include "loop/ConstantRateLoop.h"

// Helper clásico para el patrón Visitor de std::visit
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

GameLoop::GameLoop(Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor,
                   const std::filesystem::path& configDir, const WorldConfig& wConfig,
                   const ServerConfig& serverConfig):
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
        world(wConfig.worldId, wConfig.worldName, itemRegistry, characterConfigs, inventoryConfig,
              serverConfig) {
    registerHandlers();
    if (worldConfig.isNewWorld) {
        world.loadMap(worldConfig.baseMapPath, true);
    } else {
        world.loadMap(worldConfig.baseMapPath, false);
        MonsterConfigs mConfigs;
        try {
            mConfigs = MonsterConfigLoader::loadMonsterConfigs("config/monsters.toml");
        } catch (const std::exception& e) {
            std::cerr << e.what();
        }
        world.restoreMonsters(worldDataStore.loadMonsters(worldConfig.worldId), mConfigs);
        auto [npcHeaders, npcStocks] = worldDataStore.loadNpcStates(worldConfig.worldId);
        world.restoreNpcStates(npcHeaders, npcStocks);
        world.restoreGroundItems(worldDataStore.loadGroundItems(worldConfig.worldId));

        auto [clanHeaders, clanMembers, clanPending, clanBanned] =
                worldDataStore.loadClans(worldConfig.worldId);
        world.restoreClans({clanHeaders, clanMembers, clanPending, clanBanned});

        auto [bankHeaders, bankSlots] = worldDataStore.loadBankAccounts(worldConfig.worldId);
        world.restoreBank({bankHeaders, bankSlots});

        world.setUsernameCache(playerDataStore.loadAllUsernames());
    }
}

void GameLoop::registerHandlers() {
    registerCommand<StartMoveDTO>([this](uint32_t clientId, const StartMoveDTO& dto) {
        world.moveEntity(clientId, dto.direction);
    });

    registerCommand<AttackDTO>([this](uint32_t clientId, const AttackDTO& dto) {
        world.playerAttack(clientId, dto.targetId);
    });

    registerCommand<EquipItemDTO>([this](uint32_t clientId, const EquipItemDTO& dto) {
        world.equipItem(clientId, dto.slot);
    });

    registerCommand<UseItemDTO>([this](uint32_t clientId, const UseItemDTO& dto) {
        world.useItem(clientId, dto.slot);
    });

    registerCommand<DropItemDTO>([this](uint32_t clientId, const DropItemDTO& dto) {
        world.dropItem(clientId, dto.slot, dto.amount);
    });

    registerCommand<GrabItemDTO>([this](uint32_t clientId, const GrabItemDTO&) {
        world.pickUpItem(clientId);
    });

    registerCommand<MeditateDTO>([this](uint32_t clientId, const MeditateDTO&) {
        world.playerMeditate(clientId);
    });

    registerCommand<ResurrectDTO>([this](uint32_t clientId, const ResurrectDTO&) {
        world.playerResurrect(clientId);
    });

    registerCommand<SelectNpcDTO>([this](uint32_t clientId, const SelectNpcDTO& dto) {
        world.playerInteract(clientId, dto.npcId);
    });

    registerCommand<NpcCommandDTO>([this](uint32_t clientId, const NpcCommandDTO& dto) {
        world.playerExecuteNpcCommand(clientId, dto);
    });

    registerCommand<ClanCommandDTO>([this](uint32_t clientId, const ClanCommandDTO& dto) {
        world.processClanCommand(clientId, dto);
    });

    registerCommand<ChatDTO>([this](uint32_t clientId, const ChatDTO& dto) {
        auto senderName = world.getPlayerUsername(clientId);
        if (senderName.has_value()) {
            ChatDTO broadcast;
            broadcast.message = "[" + senderName.value() + "] " + dto.message;
            monitor.broadcastChat(broadcast);
        }
    });

    registerCommand<PrivateChatDTO>([this](uint32_t clientId, const PrivateChatDTO& dto) {
        auto senderName = world.getPlayerUsername(clientId);
        if (!senderName.has_value()) return;

        uint32_t recipientId = world.resolveNickToDbId(dto.recipientNick);
        if (recipientId == 0) {
            ChatDTO err;
            err.message = "[Server] Usuario '" + dto.recipientNick + "' no encontrado.";
            monitor.sendToClient(clientId, err);
            return;
        }

        ChatDTO toRecipient;
        toRecipient.message = "[PM de " + senderName.value() + "] " + dto.message;
        monitor.sendToClient(recipientId, toRecipient);

        ChatDTO toSender;
        toSender.message = "[PM → " + dto.recipientNick + "] " + dto.message;
        monitor.sendToClient(clientId, toSender);
    });

    registerCommand<ShootDTO>([this](uint32_t clientId, const ShootDTO& dto) {
        world.playerShoot(clientId, dto.targetX, dto.targetY);
    });

    registerCommand<CheatDTO>([this](uint32_t clientId, const CheatDTO& dto) {
        world.playerCheat(clientId, dto.type);
    });

    registerCommand<CreateCharacterDTO>([this](uint32_t clientId, const CreateCharacterDTO& dto) {
        auto it = pendingCreations.find(clientId);
        if (it != pendingCreations.end()) {
            std::string pendingUsername = it->second;
            pendingCreations.erase(it);

            Race race = static_cast<Race>(dto.race);
            CharacterClass cls = static_cast<CharacterClass>(dto.characterClass);
            world.addPlayer(clientId, pendingUsername, race, cls, std::nullopt);
        }
    });
}

void GameLoop::run() {
    const int MS_PER_FRAME = 33;
    ConstantRateLoop loop(MS_PER_FRAME);

    try {
        loop.run([this, MS_PER_FRAME](int64_t) {
            if (!isRunning) {
                return false;
            }

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

            return isRunning.load();
        });
        persistWorldState();  // Guardado final al salir
    } catch (const std::exception& e) {
        std::cerr << "[GAMELOOP] Error: " << e.what() << std::endl;
        persistWorldState();
        isRunning = false;
    }
}
void GameLoop::processInputs() {
    GameEvent event;

    while (gameQueue.try_pop(event)) {
        std::visit(overloaded{
            [this](const JoinEvent& joinData) {
                std::cout << "[GAMELOOP] Jugador conectado: " << joinData.username << std::endl;
                auto savedData = playerDataStore.loadPlayerData(joinData.username);
                
                if (savedData.has_value()) {
                    Race race = static_cast<Race>(savedData->race);
                    CharacterClass cls = static_cast<CharacterClass>(savedData->characterClass);
                    world.addPlayer(joinData.clientId, joinData.username, race, cls, savedData);

                    JoinResponseDTO resp;
                    resp.needsCreation = false;
                    monitor.sendToClient(joinData.clientId, resp);
                } else {
                    pendingCreations[joinData.clientId] = joinData.username;
                    JoinResponseDTO resp;
                    resp.needsCreation = true;
                    resp.baseStrength = characterConfigs.player.baseStrength;
                    resp.baseAgility = characterConfigs.player.baseAgility;
                    resp.baseIntelligence = characterConfigs.player.baseIntelligence;
                    resp.baseConstitution = characterConfigs.player.baseConstitution;

                    for (int i = 0; i < 4; ++i) {
                        auto r = static_cast<Race>(i);
                        resp.raceFactors.push_back(characterConfigs.races[r].lifeFactor);
                        resp.raceFactors.push_back(characterConfigs.races[r].manaFactor);
                        resp.raceFactors.push_back(characterConfigs.races[r].strengthFactor);
                        resp.raceFactors.push_back(characterConfigs.races[r].agilityFactor);
                        resp.raceFactors.push_back(characterConfigs.races[r].intelligenceFactor);
                    }
                    for (int i = 0; i < 4; ++i) {
                        auto c = static_cast<CharacterClass>(i);
                        resp.classFactors.push_back(characterConfigs.classes[c].lifeFactor);
                        resp.classFactors.push_back(characterConfigs.classes[c].manaFactor);
                    }
                    monitor.sendToClient(joinData.clientId, resp);
                }
            },
            [this](const DisconnectEvent& discData) {
                std::cout << "[GAMELOOP] Jugador " << discData.clientId << " solicitó desconexión." << std::endl;
                auto username = world.getPlayerUsername(discData.clientId);
                auto persistData = world.getPlayerPersistData(discData.clientId);

                if (username.has_value() && persistData.has_value()) {
                    playerDataStore.savePlayerData(username.value(), persistData.value());
                }
                world.removePlayer(discData.clientId);
                pendingCreations.erase(discData.clientId);
            },
            [this](const PlayerCommand& pCmd) {
                std::visit([this, &pCmd](const auto& concreteCmd) {
                    auto it = commandDispatcher.find(typeid(concreteCmd));
                    if (it != commandDispatcher.end()) {
                        it->second(pCmd.clientId, pCmd);
                    } else {
                        std::cerr << "[GAMELOOP] Alerta: Comando sin handler registrado." << std::endl;
                    }
                }, pCmd.command);
            }
        }, event);
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

    // Mandamos el HUD individual a cada jugador
    for (uint32_t dbId: world.getOnlinePlayerDbIds()) {
        auto statsOpt = world.getPlayerStatsDTO(dbId);
        if (statsOpt.has_value()) {
            monitor.sendToClient(dbId, statsOpt.value());
        }
    }
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
    auto [npcHeaders, npcStocks] = world.getNpcsPersistData();
    worldDataStore.saveNpcStates(worldConfig.worldId, npcHeaders, npcStocks);
    auto clanData = world.getClansPersistData();
    worldDataStore.saveClans(worldConfig.worldId, clanData.headers, clanData.members,
                             clanData.pending, clanData.banned);

    auto bankData = world.getBankPersistData();
    worldDataStore.saveBankAccounts(worldConfig.worldId, bankData.headers, bankData.slots);
}

void GameLoop::stop() { isRunning = false; }
