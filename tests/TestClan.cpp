#include <algorithm>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "../common/include/queue.h"
#include "config/CharacterConfig.h"
#include "dto/ClanCommandDTO.h"
#include "model/clan/ClanController.h"
#include "model/clan/ClanRepository.h"
#include "model/clan/ClanService.h"
#include "model/items/ItemRegistry.h"

#include "GameLoop.h"
#include "World.h"

static CharacterConfigs getTestConfigs() {
    PlayerConfig base{15, 15, 15, 15, 1, 0, 0};
    RaceConfig human{1.0f, 1.0f, 1.0f};
    CharacterClassConfig warrior{1.0f, 1.0f, 1.0f, false};
    return CharacterConfigs{base, {{Race::HUMAN, human}}, {{CharacterClass::WARRIOR, warrior}}};
}

static InventoryConfig getTestInventoryConfig() { return {16, 0, 10000, 5000}; }

struct MockWorldContext: public IWorldContext {
    uint16_t getPlayerLevel(uint32_t) const override { return 10; }
    uint32_t resolveNickToDbId(const std::string&) const override { return 0; }
    std::optional<std::string> getPlayerUsername(uint32_t dbId) const override {
        return "Player_" + std::to_string(dbId);
    }
};

class ClanSystemTest: public ::testing::Test {
protected:
    ClanRepository repo;
    ClanService service;
    ClanController controller;
    std::vector<ClanNotification> notifs;
    MockWorldContext mockCtx;

    ClanSystemTest(): service(repo, 1), controller(service) {}

    void SetUp() override {
        notifs.clear();
        service.setMinLevelToFound(1);  // Doble seguridad
    }

    // --- Helpers de Lógica (Evalúan reglas de negocio) ---
    ClanOpResult foundClan(uint32_t founder, const std::string& name, uint16_t level = 10) {
        return service.foundClan(founder, level, name);
    }
    ClanOpResult joinRequest(uint32_t player, const std::string& clan) {
        return service.joinRequest(player, clan);
    }
    ClanOpResult accept(uint32_t founder, uint32_t target) {
        return service.acceptMember(founder, target);
    }
    ClanOpResult reject(uint32_t founder, uint32_t target) {
        return service.rejectMember(founder, target);
    }
    ClanOpResult ban(uint32_t founder, uint32_t target) {
        return service.banMember(founder, target);
    }
    ClanOpResult kick(uint32_t founder, uint32_t target) {
        return service.kickMember(founder, target);
    }
    ClanOpResult leave(uint32_t player) { return service.leaveClan(player); }

    // --- Helpers de Presentación (Simulan la generación de notificaciones) ---
    void notifyFoundClan(uint32_t founder, const std::string& name, uint16_t level = 10) {
        notifs.clear();
        controller.handleFoundClan(founder, level, name, notifs);
    }
    void notifyJoinRequest(uint32_t player, const std::string& clan) {
        notifs.clear();
        controller.handleJoinRequest(player, clan, mockCtx, notifs);
    }
    void notifyAccept(uint32_t founder, uint32_t target) {
        notifs.clear();
        controller.handleAcceptMember(founder, target, "player_" + std::to_string(target), notifs);
    }
    void notifyReject(uint32_t founder, uint32_t target) {
        notifs.clear();
        controller.handleRejectMember(founder, target, "player_" + std::to_string(target), notifs);
    }
    void notifyBan(uint32_t founder, uint32_t target) {
        notifs.clear();
        controller.handleBanMember(founder, target, "player_" + std::to_string(target), notifs);
    }
    void notifyKick(uint32_t founder, uint32_t target) {
        notifs.clear();
        controller.handleKickMember(founder, target, "player_" + std::to_string(target), notifs);
    }
    void notifyLeave(uint32_t player) {
        notifs.clear();
        controller.handleLeaveClan(player, notifs);
    }

    bool hasNotifFor(uint32_t targetId, const std::string& substr) {
        return std::any_of(notifs.begin(), notifs.end(), [&](const ClanNotification& n) {
            return n.targetDbId == targetId && n.message.find(substr) != std::string::npos;
        });
    }
};
// =============================================================================
// BLOQUE 1 — Fundación de clan
// =============================================================================

TEST_F(ClanSystemTest, FoundClan_OK) {
    EXPECT_EQ(foundClan(1, "LosCampiones"), ClanOpResult::OK);
    EXPECT_TRUE(repo.getClanIdOfPlayer(1).has_value());
}

TEST_F(ClanSystemTest, FoundClan_LevelTooLow) {
    // Restauramos temporalmente la restricción de nivel a 6 solo para este test
    service.setMinLevelToFound(6);

    EXPECT_EQ(foundClan(1, "MiClan", 3), ClanOpResult::LEVEL_TOO_LOW);
    EXPECT_FALSE(repo.getClanIdOfPlayer(1).has_value());

    notifyFoundClan(1, "MiClan", 3);
    EXPECT_TRUE(hasNotifFor(1, "Necesitas más nivel"));
}

TEST_F(ClanSystemTest, FoundClan_DuplicateName) {
    foundClan(1, "Elites");
    EXPECT_EQ(foundClan(2, "Elites"), ClanOpResult::NAME_TAKEN);
    EXPECT_FALSE(repo.getClanIdOfPlayer(2).has_value());
}

TEST_F(ClanSystemTest, FoundClan_DuplicateNameCaseInsensitive) {
    foundClan(1, "Elites");
    EXPECT_EQ(foundClan(2, "ELITES"), ClanOpResult::NAME_TAKEN);
}

TEST_F(ClanSystemTest, FoundClan_PlayerAlreadyInClan) {
    foundClan(1, "Alpha");
    EXPECT_EQ(foundClan(1, "Beta"), ClanOpResult::ALREADY_IN_CLAN);
    EXPECT_TRUE(repo.getClanIdOfPlayer(1).has_value());
}

TEST_F(ClanSystemTest, FoundClan_NotificationSentToFounder) {
    notifyFoundClan(1, "LosBravos");
    EXPECT_TRUE(hasNotifFor(1, "Fundaste el clan"));
}

// =============================================================================
// BLOQUE 2 — Solicitud de unión
// =============================================================================

TEST_F(ClanSystemTest, JoinRequest_OK) {
    foundClan(1, "Alpha");
    EXPECT_EQ(joinRequest(2, "Alpha"), ClanOpResult::OK);

    notifyJoinRequest(2, "Alpha");
    EXPECT_TRUE(hasNotifFor(2, "Petición de ingreso"));
    EXPECT_TRUE(hasNotifFor(1, "pedido de ingreso"));
}

TEST_F(ClanSystemTest, JoinRequest_ClanNotFound) {
    EXPECT_EQ(joinRequest(2, "Inexistente"), ClanOpResult::CLAN_NOT_FOUND);

    notifyJoinRequest(2, "Inexistente");
    EXPECT_TRUE(hasNotifFor(2, "no existe"));
}

TEST_F(ClanSystemTest, JoinRequest_AlreadyInClan) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);
    foundClan(3, "Beta");
    EXPECT_EQ(joinRequest(2, "Beta"), ClanOpResult::ALREADY_IN_CLAN);
}

TEST_F(ClanSystemTest, JoinRequest_Banned) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    ban(1, 2);
    EXPECT_EQ(joinRequest(2, "Alpha"), ClanOpResult::PLAYER_BANNED);
}

TEST_F(ClanSystemTest, JoinRequest_ClanFull) {
    foundClan(1, "FullClan");
    for (uint32_t id = 2; id <= CLAN_MAX_MEMBERS; ++id) {
        joinRequest(id, "FullClan");
        accept(1, id);
    }
    EXPECT_EQ(joinRequest(100, "FullClan"), ClanOpResult::CLAN_FULL);
}

// =============================================================================
// BLOQUE 3 — Aceptar / Rechazar / Ban / Kick
// =============================================================================

TEST_F(ClanSystemTest, AcceptMember_OK) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    EXPECT_EQ(accept(1, 2), ClanOpResult::OK);
    EXPECT_TRUE(repo.getClanIdOfPlayer(2).has_value());
    EXPECT_EQ(repo.getClanIdOfPlayer(2), repo.getClanIdOfPlayer(1));
}

TEST_F(ClanSystemTest, AcceptMember_NotFounder) {
    foundClan(1, "Alpha");
    joinRequest(3, "Alpha");
    accept(1, 3);
    joinRequest(4, "Alpha");
    EXPECT_EQ(accept(3, 4), ClanOpResult::NOT_FOUNDER);
}

TEST_F(ClanSystemTest, AcceptMember_NoPendingRequest) {
    foundClan(1, "Alpha");
    EXPECT_EQ(accept(1, 2), ClanOpResult::NO_PENDING_REQUEST);
}

TEST_F(ClanSystemTest, AcceptMember_NotificationsAreSent) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    notifyAccept(1, 2);
    EXPECT_TRUE(hasNotifFor(2, "aceptado en el clan"));
}

TEST_F(ClanSystemTest, RejectMember_OK) {
    notifyFoundClan(1, "Alpha");
    notifyJoinRequest(2, "Alpha");
    notifyReject(1, 2);
    EXPECT_FALSE(repo.getClanIdOfPlayer(2).has_value());
    EXPECT_TRUE(hasNotifFor(2, "rechazada"));
}

TEST_F(ClanSystemTest, BanMember_PreventsFutureRequests) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    ban(1, 2);
    EXPECT_EQ(joinRequest(2, "Alpha"), ClanOpResult::PLAYER_BANNED);

    notifyBan(1, 2);
    EXPECT_TRUE(hasNotifFor(2, "baneado"));
}

TEST_F(ClanSystemTest, BanMember_AlsoRemovesActiveMember) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);
    ban(1, 2);
    EXPECT_FALSE(repo.getClanIdOfPlayer(2).has_value());
}

TEST_F(ClanSystemTest, KickMember_OK) {
    notifyFoundClan(1, "Alpha");
    notifyJoinRequest(2, "Alpha");
    notifyAccept(1, 2);
    notifyKick(1, 2);
    EXPECT_FALSE(repo.getClanIdOfPlayer(2).has_value());
    EXPECT_TRUE(hasNotifFor(2, "expulsado"));
}

TEST_F(ClanSystemTest, KickMember_CannotKickSelf) {
    foundClan(1, "Alpha");
    EXPECT_EQ(kick(1, 1), ClanOpResult::NOT_FOUNDER);
}

TEST_F(ClanSystemTest, KickMember_UnlikeKick_DoesNotBan) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);
    kick(1, 2);
    EXPECT_EQ(joinRequest(2, "Alpha"), ClanOpResult::OK);
}

// =============================================================================
// BLOQUE 4 — Dejar el clan
// =============================================================================

TEST_F(ClanSystemTest, LeaveClan_MemberOK) {
    notifyFoundClan(1, "Alpha");
    notifyJoinRequest(2, "Alpha");
    notifyAccept(1, 2);
    notifyLeave(2);
    EXPECT_FALSE(repo.getClanIdOfPlayer(2).has_value());
    EXPECT_TRUE(hasNotifFor(2, "Abandonaste el clan"));
}

TEST_F(ClanSystemTest, LeaveClan_FounderCannot) {
    foundClan(1, "Alpha");
    EXPECT_EQ(leave(1), ClanOpResult::CANNOT_LEAVE_AS_FOUNDER);
    EXPECT_TRUE(repo.getClanIdOfPlayer(1).has_value());
}

TEST_F(ClanSystemTest, LeaveClan_NotInClan) { EXPECT_EQ(leave(99), ClanOpResult::NOT_IN_CLAN); }

// =============================================================================
// BLOQUE 5 — Revisar clan
// =============================================================================

TEST_F(ClanSystemTest, ReviewClan_ContainsMemberList) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);

    notifs.clear();
    controller.handleReviewClan(1, mockCtx, notifs);
    EXPECT_TRUE(hasNotifFor(1, "Alpha"));
    EXPECT_TRUE(hasNotifFor(1, "Líder"));
}

TEST_F(ClanSystemTest, ReviewClan_ShowsPendingRequests) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");

    notifs.clear();
    controller.handleReviewClan(1, mockCtx, notifs);
    EXPECT_TRUE(hasNotifFor(1, std::to_string(2)));  // ID de la solicitud pendiente
}

TEST_F(ClanSystemTest, ReviewClan_OnlyFounderCanReview) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);

    notifs.clear();
    controller.handleReviewClan(2, mockCtx, notifs);
    EXPECT_TRUE(hasNotifFor(2, "Solo el fundador"));
}

// =============================================================================
// BLOQUE 6 — Integración con World
// =============================================================================

class WorldClanTest: public ::testing::Test {
protected:
    ItemRegistry* registry = nullptr;
    World* world = nullptr;
    PlayerPersistData pdata{};

    void SetUp() override {
        registry = new ItemRegistry("../config/items.toml");
        CharacterConfigs configs = getTestConfigs();
        world = new World(1, "Tester", *registry, configs, getTestInventoryConfig());

        // Desactivamos el Fair Play (Modo Arena) y bajamos el nivel de clan a 1 para los tests
        world->setFairPlayRules(false);
        world->setClanMinLevel(1);

        pdata.dbId = 1;
        pdata.posX = 5;
        pdata.posY = 5;
        pdata.level = 1;
        pdata.hp = 15;
        pdata.mana = 15;

        pdata.race = static_cast<uint8_t>(Race::HUMAN);
        pdata.characterClass = static_cast<uint8_t>(CharacterClass::WARRIOR);

        std::string u1 = "Founder", u2 = "Member1", u3 = "Member2";

        // Pasamos el mismo pdata limpio para los tres (les cambia el dbId adentro de addPlayer)
        world->addPlayer(1, u1, Race::HUMAN, CharacterClass::WARRIOR, pdata);
        world->addPlayer(2, u2, Race::HUMAN, CharacterClass::WARRIOR, pdata);
        world->addPlayer(3, u3, Race::HUMAN, CharacterClass::WARRIOR, pdata);
    }

    void TearDown() override {
        delete world;
        delete registry;
    }

    // Envía un ClanCommandDTO al World y descarta notificaciones intermedias
    void sendCmd(uint32_t sender, ClanCommandType type, const std::string& arg = "",
                 uint32_t targetDbId = 0) {
        ClanCommandDTO cmd;
        cmd.type = type;
        cmd.arg1 = arg;
        cmd.targetDbId = targetDbId;
        world->processClanCommand(sender, cmd);
        world->pollEvents();  // limpiar cola de eventos
    }

    // Funda un clan con el jugador 1 y hace que 2 y 3 sean miembros
    void setupClanWithMembers() {
        sendCmd(1, ClanCommandType::FOUND, "Alpha");
        sendCmd(2, ClanCommandType::JOIN, "Alpha");
        sendCmd(1, ClanCommandType::ACCEPT, "Member1", 2);
        sendCmd(3, ClanCommandType::JOIN, "Alpha");
        sendCmd(1, ClanCommandType::ACCEPT, "Member2", 3);
    }
};

TEST_F(WorldClanTest, World_FoundClan_EventReachesQueue) {
    sendCmd(1, ClanCommandType::FOUND, "LosBravos");
    // No lanzó excepción: el comando se procesó sin errores
    SUCCEED();
}

TEST_F(WorldClanTest, World_ClanmatesCannotAttackEachOther) {
    setupClanWithMembers();

    // Intentar atacar: jugador 1 a jugador 2 (clanmates) → debe generar evento de rechazo
    world->playerAttack(1, 2);
    auto events = world->pollEvents();

    bool rejectedFound = std::any_of(events.begin(), events.end(), [](const WorldEvent& e) {
        return e.message.find("No puedes atacar") != std::string::npos;
    });
    EXPECT_TRUE(rejectedFound);
}

TEST_F(WorldClanTest, World_PlayersInDifferentClans_CanAttackEachOther) {
    // Jugador 1 funda Alpha, jugador 3 funda Beta
    sendCmd(1, ClanCommandType::FOUND, "Alpha");

    // Agregar cuarto jugador para fundar Beta
    std::string u4 = "OtherFounder";
    world->addPlayer(4, u4, Race::HUMAN, CharacterClass::WARRIOR, pdata);
    world->pollEvents();

    sendCmd(4, ClanCommandType::FOUND, "Beta");

    // 1 ataca a 4 (clanes distintos): no debe haber mensaje de "No puedes atacar"
    world->playerAttack(1, 4);
    auto events = world->pollEvents();

    bool rejectedFound = std::any_of(events.begin(), events.end(), [](const WorldEvent& e) {
        return e.message.find("No puedes atacar") != std::string::npos;
    });
    EXPECT_FALSE(rejectedFound);
}

TEST_F(WorldClanTest, World_UnderAttack_NotifiesClanmates) {
    setupClanWithMembers();

    // Agregar un cuarto jugador sin clan para que ataque al clan
    std::string u4 = "Enemy";
    world->addPlayer(4, u4, Race::HUMAN, CharacterClass::WARRIOR, pdata);
    world->pollEvents();

    // Enemy (4) ataca a Member1 (2)
    world->playerAttack(4, 2);
    auto events = world->pollEvents();

    // Los clanmates (1 y 3) deben recibir la alerta
    bool notif1 = std::any_of(events.begin(), events.end(), [](const WorldEvent& e) {
        return e.targetDbId == 1 && e.message.find("atacado") != std::string::npos;
    });
    bool notif3 = std::any_of(events.begin(), events.end(), [](const WorldEvent& e) {
        return e.targetDbId == 3 && e.message.find("atacado") != std::string::npos;
    });

    EXPECT_TRUE(notif1);
    EXPECT_TRUE(notif3);
}

TEST_F(WorldClanTest, World_LoginNotifiesClanmates) {
    setupClanWithMembers();

    // Un nuevo jugador se une al clan
    std::string u5 = "Newbie";
    world->addPlayer(5, u5, Race::HUMAN, CharacterClass::WARRIOR,
                     pdata);  // esto genera notificación si 5 ya estuviera en un clan
    // En este caso 5 no está en un clan aún, así que no hay notificación de clan en login
    auto events = world->pollEvents();

    bool unexpectedClanNotif = std::any_of(events.begin(), events.end(), [](const WorldEvent& e) {
        return e.message.find("[Clan]") != std::string::npos &&
               e.message.find("Newbie") != std::string::npos;
    });
    EXPECT_FALSE(unexpectedClanNotif);
}

TEST_F(WorldClanTest, World_LogoffNotifiesClanmates) {
    setupClanWithMembers();

    // Jugador 2 (miembro del clan) se desconecta
    world->removePlayer(2);
    auto events = world->pollEvents();

    // Jugador 1 debe recibir notificación de logoff de Member1
    bool foundLogoff = std::any_of(events.begin(), events.end(), [](const WorldEvent& e) {
        return e.targetDbId == 1 && e.message.find("[Clan]") != std::string::npos &&
               e.message.find("salió") != std::string::npos;
    });
    EXPECT_TRUE(foundLogoff);
}

TEST_F(WorldClanTest, World_AreClanmates_TrueAfterAccept) {
    setupClanWithMembers();
    EXPECT_TRUE(world->areClanmates(1, 2));
    EXPECT_TRUE(world->areClanmates(2, 3));
}

TEST_F(WorldClanTest, World_AreClanmates_FalseForNonMembers) {
    sendCmd(1, ClanCommandType::FOUND, "Alpha");
    // Jugador 2 y 3 no están en el clan
    EXPECT_FALSE(world->areClanmates(1, 2));
    EXPECT_FALSE(world->areClanmates(2, 3));
}

TEST_F(WorldClanTest, World_ProcessClanCommand_LeaveRemovesMember) {
    setupClanWithMembers();
    sendCmd(2, ClanCommandType::LEAVE);
    // Ya no son clanmates
    EXPECT_FALSE(world->areClanmates(1, 2));
}

TEST_F(WorldClanTest, World_ProcessClanCommand_FounderCannotLeave) {
    setupClanWithMembers();

    // Capturar el mensaje de error
    ClanCommandDTO cmd;
    cmd.type = ClanCommandType::LEAVE;
    world->processClanCommand(1, cmd);
    auto events = world->pollEvents();

    bool foundError = std::any_of(events.begin(), events.end(), [](const WorldEvent& e) {
        return e.targetDbId == 1 && e.message.find("fundador") != std::string::npos;
    });
    EXPECT_TRUE(foundError);
    // Sigue siendo clanmate con 2
    EXPECT_TRUE(world->areClanmates(1, 2));
}

// =============================================================================
// BLOQUE 7 — End-to-end GameLoop con ClanCommandDTO
// =============================================================================

TEST(ClanGameLoopTest, GameLoop_ProcessesClanFoundCommand) {
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;
    WorldConfig wConfig{1, "Test", "maps/defaultMap.json", "game_data/", true};
    GameLoop loop(gameQueue, monitor, "../config", wConfig);
    // Jugador ingresa
    JoinEvent join;
    join.clientId = 1;
    join.username = "HeroFounder";
    gameQueue.push(join);

    // El jugador envía /fundar-clan
    ClanCommandDTO clanCmd;
    clanCmd.type = ClanCommandType::FOUND;
    clanCmd.arg1 = "LosHeroes";

    PlayerCommand pCmd;
    pCmd.clientId = 1;
    pCmd.command = clanCmd;
    gameQueue.push(pCmd);

    std::thread hiloGL(&GameLoop::run, &loop);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    loop.stop();
    if (hiloGL.joinable())
        hiloGL.join();

    // Si no lanzó excepción, el comando fue despachado correctamente
    SUCCEED();
}

TEST(ClanGameLoopTest, GameLoop_ProcessesClanJoinAndAccept) {
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;
    WorldConfig wConfig{1, "Test", "maps/defaultMap.json", "game_data/", true};
    GameLoop loop(gameQueue, monitor, "../config", wConfig);

    // Dos jugadores ingresan
    auto pushJoin = [&](uint32_t id, const std::string& name) {
        JoinEvent j;
        j.clientId = id;
        j.username = name;
        gameQueue.push(j);
    };
    auto pushClan = [&](uint32_t id, ClanCommandType type, const std::string& arg = "",
                        uint32_t target = 0) {
        ClanCommandDTO c;
        c.type = type;
        c.arg1 = arg;
        c.targetDbId = target;
        PlayerCommand p;
        p.clientId = id;
        p.command = c;
        gameQueue.push(p);
    };

    pushJoin(1, "Lider");
    pushJoin(2, "Soldado");
    pushClan(1, ClanCommandType::FOUND, "GuardiaReal");
    pushClan(2, ClanCommandType::JOIN, "GuardiaReal");
    pushClan(1, ClanCommandType::ACCEPT, "Soldado", 2);

    std::thread hiloGL(&GameLoop::run, &loop);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    loop.stop();
    if (hiloGL.joinable())
        hiloGL.join();

    SUCCEED();
}
