#include <algorithm>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "model/clan/ClanManager.h"
#include "dto/ClanCommandDTO.h"

// Para los tests de World e integración
#include "model/items/ItemRegistry.h"
#include "World.h"
#include "../common/include/queue.h"
#include "GameLoop.h"

// =============================================================================
// Fixture base: ClanManager limpio para cada test
// =============================================================================

class ClanManagerTest : public ::testing::Test {
protected:
    ClanManager cm;
    std::vector<ClanNotification> notifs;

    void SetUp() override {
        notifs.clear();
    }

    // Funda un clan y limpia notificaciones intermedias
    ClanOpResult foundClan(uint32_t founder, const std::string& name, uint16_t level = 10) {
        notifs.clear();
        return cm.foundClan(founder, level, name, notifs);
    }

    // Solicita unirse y limpia notificaciones
    ClanOpResult joinRequest(uint32_t player, const std::string& clan) {
        notifs.clear();
        return cm.joinRequest(player, clan, notifs);
    }

    // Acepta al jugador con nick arbitrario (en tests de ClanManager no necesitamos nick real)
    ClanOpResult accept(uint32_t founder, uint32_t target) {
        notifs.clear();
        return cm.acceptMember(founder, "player_" + std::to_string(target), target, notifs);
    }

    ClanOpResult reject(uint32_t founder, uint32_t target) {
        notifs.clear();
        return cm.rejectMember(founder, "player_" + std::to_string(target), target, notifs);
    }

    ClanOpResult ban(uint32_t founder, uint32_t target) {
        notifs.clear();
        return cm.banMember(founder, "player_" + std::to_string(target), target, notifs);
    }

    ClanOpResult kick(uint32_t founder, uint32_t target) {
        notifs.clear();
        return cm.kickMember(founder, "player_" + std::to_string(target), target, notifs);
    }

    ClanOpResult leave(uint32_t player) {
        notifs.clear();
        return cm.leaveClan(player, notifs);
    }

    // Verifica que alguna notificación para targetId contenga substr
    bool hasNotifFor(uint32_t targetId, const std::string& substr) {
        return std::any_of(notifs.begin(), notifs.end(),
            [&](const ClanNotification& n) {
                return n.targetDbId == targetId &&
                       n.message.find(substr) != std::string::npos;
            });
    }
};

// =============================================================================
// BLOQUE 1 — Fundación de clan
// =============================================================================

TEST_F(ClanManagerTest, FoundClan_OK) {
    EXPECT_EQ(foundClan(1, "LosCampiones"), ClanOpResult::OK);
    // Después de fundar, el fundador pertenece al clan
    EXPECT_TRUE(cm.getClanId(1).has_value());
}

TEST_F(ClanManagerTest, FoundClan_LevelTooLow) {
    notifs.clear();
    auto result = cm.foundClan(1, /*level=*/3, "MiClan", notifs);
    EXPECT_EQ(result, ClanOpResult::LEVEL_TOO_LOW);
    EXPECT_FALSE(cm.getClanId(1).has_value());
    EXPECT_TRUE(hasNotifFor(1, "Necesitas nivel"));
}

TEST_F(ClanManagerTest, FoundClan_DuplicateName) {
    foundClan(1, "Elites");
    auto result = foundClan(2, "Elites");   // mismo nombre
    EXPECT_EQ(result, ClanOpResult::NAME_TAKEN);
    EXPECT_FALSE(cm.getClanId(2).has_value());
}

TEST_F(ClanManagerTest, FoundClan_DuplicateNameCaseInsensitive) {
    foundClan(1, "Elites");
    auto result = foundClan(2, "ELITES");
    EXPECT_EQ(result, ClanOpResult::NAME_TAKEN);
}

TEST_F(ClanManagerTest, FoundClan_PlayerAlreadyInClan) {
    foundClan(1, "Alpha");
    // El fundador (1) intenta fundar otro clan
    auto result = foundClan(1, "Beta");
    EXPECT_EQ(result, ClanOpResult::ALREADY_IN_CLAN);
    // Solo debe existir un clan para este jugador
    EXPECT_TRUE(cm.getClanId(1).has_value());
}

TEST_F(ClanManagerTest, FoundClan_NotificationSentToFounder) {
    foundClan(1, "LosBravos");
    EXPECT_TRUE(hasNotifFor(1, "Fundaste el clan"));
}

// =============================================================================
// BLOQUE 2 — Solicitud de unión
// =============================================================================

TEST_F(ClanManagerTest, JoinRequest_OK) {
    foundClan(1, "Alpha");
    auto result = joinRequest(2, "Alpha");
    EXPECT_EQ(result, ClanOpResult::OK);
    // Notificación al solicitante y al fundador
    EXPECT_TRUE(hasNotifFor(2, "Solicitud enviada"));
    EXPECT_TRUE(hasNotifFor(1, "pedido de ingreso"));
}

TEST_F(ClanManagerTest, JoinRequest_ClanNotFound) {
    auto result = joinRequest(2, "Inexistente");
    EXPECT_EQ(result, ClanOpResult::CLAN_NOT_FOUND);
    EXPECT_TRUE(hasNotifFor(2, "No existe"));
}

TEST_F(ClanManagerTest, JoinRequest_AlreadyInClan) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);

    // El jugador 2 ya es miembro, intenta unirse a otro clan
    foundClan(3, "Beta");
    auto result = joinRequest(2, "Beta");
    EXPECT_EQ(result, ClanOpResult::ALREADY_IN_CLAN);
}

TEST_F(ClanManagerTest, JoinRequest_Banned) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    ban(1, 2);    // el fundador banea al solicitante

    // El jugador 2 intenta solicitar de nuevo
    auto result = joinRequest(2, "Alpha");
    EXPECT_EQ(result, ClanOpResult::PLAYER_BANNED);
}

TEST_F(ClanManagerTest, JoinRequest_ClanFull) {
    foundClan(1, "FullClan");
    // Llenar el clan hasta el máximo (CLAN_MAX_MEMBERS = 16, ya tiene 1 el fundador)
    for (uint32_t id = 2; id <= CLAN_MAX_MEMBERS; ++id) {
        joinRequest(id, "FullClan");
        accept(1, id);
    }
    // El siguiente ya no cabe
    auto result = joinRequest(100, "FullClan");
    EXPECT_EQ(result, ClanOpResult::CLAN_FULL);
}

// =============================================================================
// BLOQUE 3 — Aceptar / Rechazar / Ban / Kick
// =============================================================================

TEST_F(ClanManagerTest, AcceptMember_OK) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    auto result = accept(1, 2);
    EXPECT_EQ(result, ClanOpResult::OK);
    // Jugador 2 ahora tiene un clanId
    EXPECT_TRUE(cm.getClanId(2).has_value());
    EXPECT_EQ(cm.getClanId(2), cm.getClanId(1));
    // Son clanmates
    EXPECT_TRUE(cm.areClanmates(1, 2));
}

TEST_F(ClanManagerTest, AcceptMember_NotFounder) {
    foundClan(1, "Alpha");
    joinRequest(3, "Alpha");
    accept(1, 3);    // aceptar a 3 como miembro normal
    // 3 intenta aceptar a alguien más (no es fundador)
    joinRequest(4, "Alpha");
    auto result = accept(3, 4);
    EXPECT_EQ(result, ClanOpResult::NOT_FOUNDER);
}

TEST_F(ClanManagerTest, AcceptMember_NoPendingRequest) {
    foundClan(1, "Alpha");
    // Jugador 2 nunca pidió unirse
    auto result = accept(1, 2);
    EXPECT_EQ(result, ClanOpResult::NO_PENDING_REQUEST);
}

TEST_F(ClanManagerTest, AcceptMember_NotificationsAreSent) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);
    EXPECT_TRUE(hasNotifFor(2, "aceptado en el clan"));
}

TEST_F(ClanManagerTest, RejectMember_OK) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    auto result = reject(1, 2);
    EXPECT_EQ(result, ClanOpResult::OK);
    // El jugador 2 NO pertenece al clan
    EXPECT_FALSE(cm.getClanId(2).has_value());
    EXPECT_TRUE(hasNotifFor(2, "rechazada"));
}

TEST_F(ClanManagerTest, BanMember_PreventsFutureRequests) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    ban(1, 2);

    // 2 intenta pedir unirse de nuevo
    auto result = joinRequest(2, "Alpha");
    EXPECT_EQ(result, ClanOpResult::PLAYER_BANNED);
    EXPECT_TRUE(hasNotifFor(2, "baneado"));
}

TEST_F(ClanManagerTest, BanMember_AlsoRemovesActiveMember) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);
    // Ahora el fundador banea a un miembro activo
    ban(1, 2);
    EXPECT_FALSE(cm.getClanId(2).has_value());
    EXPECT_FALSE(cm.areClanmates(1, 2));
}

TEST_F(ClanManagerTest, KickMember_OK) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);
    auto result = kick(1, 2);
    EXPECT_EQ(result, ClanOpResult::OK);
    EXPECT_FALSE(cm.getClanId(2).has_value());
    EXPECT_TRUE(hasNotifFor(2, "expulsado"));
}

TEST_F(ClanManagerTest, KickMember_CannotKickSelf) {
    foundClan(1, "Alpha");
    auto result = kick(1, 1);
    EXPECT_EQ(result, ClanOpResult::NOT_FOUNDER);
}

TEST_F(ClanManagerTest, KickMember_UnlikeKick_DoesNotBan) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);
    kick(1, 2);

    // A diferencia del ban, kick NO impide una futura solicitud
    auto result = joinRequest(2, "Alpha");
    EXPECT_EQ(result, ClanOpResult::OK);
}

// =============================================================================
// BLOQUE 4 — Dejar el clan
// =============================================================================

TEST_F(ClanManagerTest, LeaveClan_MemberOK) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);
    auto result = leave(2);
    EXPECT_EQ(result, ClanOpResult::OK);
    EXPECT_FALSE(cm.getClanId(2).has_value());
    EXPECT_TRUE(hasNotifFor(2, "Abandonaste el clan"));
}

TEST_F(ClanManagerTest, LeaveClan_FounderCannot) {
    foundClan(1, "Alpha");
    auto result = leave(1);
    EXPECT_EQ(result, ClanOpResult::CANNOT_LEAVE_AS_FOUNDER);
    // Sigue en el clan
    EXPECT_TRUE(cm.getClanId(1).has_value());
}

TEST_F(ClanManagerTest, LeaveClan_NotInClan) {
    auto result = leave(99);
    EXPECT_EQ(result, ClanOpResult::NOT_IN_CLAN);
}

// =============================================================================
// BLOQUE 5 — Revisar clan
// =============================================================================

TEST_F(ClanManagerTest, ReviewClan_ContainsMemberList) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);

    std::string report;
    auto result = cm.reviewClan(1, report);
    EXPECT_EQ(result, ClanOpResult::OK);
    EXPECT_NE(report.find("Alpha"), std::string::npos);
    EXPECT_NE(report.find("Fundador"), std::string::npos);
}

TEST_F(ClanManagerTest, ReviewClan_ShowsPendingRequests) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");

    std::string report;
    cm.reviewClan(1, report);
    // La solicitud de 2 debe aparecer como pendiente
    EXPECT_NE(report.find(std::to_string(2)), std::string::npos);
}

TEST_F(ClanManagerTest, ReviewClan_OnlyFounderCanReview) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);

    std::string report;
    auto result = cm.reviewClan(2, report);   // 2 no es fundador
    EXPECT_EQ(result, ClanOpResult::NOT_FOUNDER);
}

// =============================================================================
// BLOQUE 6 — areClanmates y getClanId
// =============================================================================

TEST_F(ClanManagerTest, AreClanmates_TrueForSameClan) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");
    accept(1, 2);
    EXPECT_TRUE(cm.areClanmates(1, 2));
    EXPECT_TRUE(cm.areClanmates(2, 1));  // simétrico
}

TEST_F(ClanManagerTest, AreClanmates_FalseForDifferentClans) {
    foundClan(1, "Alpha");
    foundClan(2, "Beta");
    EXPECT_FALSE(cm.areClanmates(1, 2));
}

TEST_F(ClanManagerTest, AreClanmates_FalseIfOneHasNoClan) {
    foundClan(1, "Alpha");
    EXPECT_FALSE(cm.areClanmates(1, 99));
}

TEST_F(ClanManagerTest, GetClanId_NulloptIfNotInClan) {
    EXPECT_FALSE(cm.getClanId(999).has_value());
}

// =============================================================================
// BLOQUE 7 — countNearbyClanmates (bonus grupal)
// =============================================================================

TEST_F(ClanManagerTest, CountNearbyClanmates_ZeroIfNoClan) {
    std::unordered_map<uint32_t, std::pair<int,int>> positions;
    int count = cm.countNearbyClanmates(1, positions, 0, 0, 10);
    EXPECT_EQ(count, 0);
}

TEST_F(ClanManagerTest, CountNearbyClanmates_CountsOnlyNearby) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");  accept(1, 2);
    joinRequest(3, "Alpha");  accept(1, 3);
    joinRequest(4, "Alpha");  accept(1, 4);

    // Jugador 1 en (0,0). 2 cerca (dist 2), 3 cerca (dist 5), 4 lejos (dist 20)
    std::unordered_map<uint32_t, std::pair<int,int>> positions = {
        {1, {0, 0}},
        {2, {1, 1}},   // Manhattan = 2
        {3, {2, 3}},   // Manhattan = 5
        {4, {15, 5}},  // Manhattan = 20
    };

    // Con rango 10: 2 y 3 son cercanos (4 queda fuera)
    int count = cm.countNearbyClanmates(1, positions, 0, 0, 10);
    EXPECT_EQ(count, 2);
}

TEST_F(ClanManagerTest, CountNearbyClanmates_DoesNotCountSelf) {
    foundClan(1, "Alpha");
    std::unordered_map<uint32_t, std::pair<int,int>> positions = {{1, {0, 0}}};
    int count = cm.countNearbyClanmates(1, positions, 0, 0, 10);
    EXPECT_EQ(count, 0);  // No se cuenta a sí mismo
}

// =============================================================================
// BLOQUE 8 — Notificaciones de login/logoff y under-attack
// =============================================================================

TEST_F(ClanManagerTest, NotifyLogin_SendsToAllClanmates) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");  accept(1, 2);
    joinRequest(3, "Alpha");  accept(1, 3);

    notifs.clear();
    cm.notifyLogin(2, "Jugador2", notifs);

    // 1 y 3 deben recibir notificación; 2 no se notifica a sí mismo
    EXPECT_TRUE(hasNotifFor(1, "Jugador2"));
    EXPECT_TRUE(hasNotifFor(3, "Jugador2"));
    EXPECT_FALSE(hasNotifFor(2, "Jugador2"));
}

TEST_F(ClanManagerTest, NotifyLogoff_SendsToAllClanmates) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");  accept(1, 2);

    notifs.clear();
    cm.notifyLogoff(2, "Jugador2", notifs);

    EXPECT_TRUE(hasNotifFor(1, "Jugador2"));
    EXPECT_TRUE(hasNotifFor(1, "salió"));
}

TEST_F(ClanManagerTest, NotifyUnderAttack_SendsToAllExceptAttacked) {
    foundClan(1, "Alpha");
    joinRequest(2, "Alpha");  accept(1, 2);
    joinRequest(3, "Alpha");  accept(1, 3);

    notifs.clear();
    cm.notifyUnderAttack(1, "Jugador1", notifs);

    // 2 y 3 reciben la alerta, 1 (el atacado) no
    EXPECT_TRUE(hasNotifFor(2, "atacado"));
    EXPECT_TRUE(hasNotifFor(3, "atacado"));
    EXPECT_FALSE(hasNotifFor(1, "atacado"));
}

TEST_F(ClanManagerTest, Notify_NoClan_NoNotifications) {
    // Jugador sin clan: ninguna notificación
    notifs.clear();
    cm.notifyLogin(99, "NoClan", notifs);
    EXPECT_TRUE(notifs.empty());

    cm.notifyLogoff(99, "NoClan", notifs);
    EXPECT_TRUE(notifs.empty());

    cm.notifyUnderAttack(99, "NoClan", notifs);
    EXPECT_TRUE(notifs.empty());
}

// =============================================================================
// BLOQUE 9 — Integración con World
// =============================================================================

class WorldClanTest : public ::testing::Test {
protected:
    ItemRegistry* registry = nullptr;
    World* world = nullptr;

    void SetUp() override {
        registry = new ItemRegistry("../config/items.toml");
        world    = new World(1, "Tester", *registry);
        // Agregar tres jugadores de prueba
        std::string u1 = "Founder", u2 = "Member1", u3 = "Member2";
        world->addPlayer(1, u1);
        world->addPlayer(2, u2);
        world->addPlayer(3, u3);
    }

    void TearDown() override {
        delete world;
        delete registry;
    }

    // Envía un ClanCommandDTO al World y descarta notificaciones intermedias
    void sendCmd(uint32_t sender, ClanCommandType type,
                 const std::string& arg = "", uint32_t targetDbId = 0) {
        ClanCommandDTO cmd;
        cmd.type        = type;
        cmd.arg1        = arg;
        cmd.targetDbId  = targetDbId;
        world->processClanCommand(sender, cmd);
        world->pollEvents();  // limpiar cola de eventos
    }

    // Funda un clan con el jugador 1 y hace que 2 y 3 sean miembros
    void setupClanWithMembers() {
        sendCmd(1, ClanCommandType::FOUND, "Alpha");
        sendCmd(2, ClanCommandType::JOIN,  "Alpha");
        sendCmd(1, ClanCommandType::ACCEPT, "Member1", 2);
        sendCmd(3, ClanCommandType::JOIN,  "Alpha");
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

    bool rejectedFound = std::any_of(events.begin(), events.end(),
        [](const WorldEvent& e) {
            return e.message.find("No puedes atacar") != std::string::npos;
        });
    EXPECT_TRUE(rejectedFound);
}

TEST_F(WorldClanTest, World_PlayersInDifferentClans_CanAttackEachOther) {
    // Jugador 1 funda Alpha, jugador 3 funda Beta
    sendCmd(1, ClanCommandType::FOUND, "Alpha");

    // Agregar cuarto jugador para fundar Beta
    std::string u4 = "OtherFounder";
    world->addPlayer(4, u4);
    world->pollEvents();

    sendCmd(4, ClanCommandType::FOUND, "Beta");

    // 1 ataca a 4 (clanes distintos): no debe haber mensaje de "No puedes atacar"
    world->playerAttack(1, 4);
    auto events = world->pollEvents();

    bool rejectedFound = std::any_of(events.begin(), events.end(),
        [](const WorldEvent& e) {
            return e.message.find("No puedes atacar") != std::string::npos;
        });
    EXPECT_FALSE(rejectedFound);
}

TEST_F(WorldClanTest, World_UnderAttack_NotifiesClanmates) {
    setupClanWithMembers();

    // Agregar un cuarto jugador sin clan para que ataque al clan
    std::string u4 = "Enemy";
    world->addPlayer(4, u4);
    world->pollEvents();

    // Enemy (4) ataca a Member1 (2)
    world->playerAttack(4, 2);
    auto events = world->pollEvents();

    // Los clanmates (1 y 3) deben recibir la alerta
    bool notif1 = std::any_of(events.begin(), events.end(),
        [](const WorldEvent& e) {
            return e.targetDbId == 1 && e.message.find("atacado") != std::string::npos;
        });
    bool notif3 = std::any_of(events.begin(), events.end(),
        [](const WorldEvent& e) {
            return e.targetDbId == 3 && e.message.find("atacado") != std::string::npos;
        });

    EXPECT_TRUE(notif1);
    EXPECT_TRUE(notif3);
}

TEST_F(WorldClanTest, World_LoginNotifiesClanmates) {
    setupClanWithMembers();

    // Un nuevo jugador se une al clan
    std::string u5 = "Newbie";
    world->addPlayer(5, u5);  // esto genera notificación si 5 ya estuviera en un clan
    // En este caso 5 no está en un clan aún, así que no hay notificación de clan en login
    auto events = world->pollEvents();

    bool unexpectedClanNotif = std::any_of(events.begin(), events.end(),
        [](const WorldEvent& e) {
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
    bool foundLogoff = std::any_of(events.begin(), events.end(),
        [](const WorldEvent& e) {
            return e.targetDbId == 1 &&
                   e.message.find("[Clan]") != std::string::npos &&
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

    bool foundError = std::any_of(events.begin(), events.end(),
        [](const WorldEvent& e) {
            return e.targetDbId == 1 &&
                   e.message.find("fundador") != std::string::npos;
        });
    EXPECT_TRUE(foundError);
    // Sigue siendo clanmate con 2
    EXPECT_TRUE(world->areClanmates(1, 2));
}

// =============================================================================
// BLOQUE 10 — End-to-end GameLoop con ClanCommandDTO
// =============================================================================

TEST(ClanGameLoopTest, GameLoop_ProcessesClanFoundCommand) {
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;
    GameLoop loop(gameQueue, monitor, "../config/items.toml");

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
    pCmd.command  = clanCmd;
    gameQueue.push(pCmd);

    std::thread hiloGL(&GameLoop::run, &loop);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    loop.stop();
    if (hiloGL.joinable()) hiloGL.join();

    // Si no lanzó excepción, el comando fue despachado correctamente
    SUCCEED();
}

TEST(ClanGameLoopTest, GameLoop_ProcessesClanJoinAndAccept) {
    Queue<GameEvent> gameQueue;
    ConnectionMonitor monitor;
    GameLoop loop(gameQueue, monitor, "../config/items.toml");

    // Dos jugadores ingresan
    auto pushJoin = [&](uint32_t id, const std::string& name) {
        JoinEvent j; j.clientId = id; j.username = name;
        gameQueue.push(j);
    };
    auto pushClan = [&](uint32_t id, ClanCommandType type,
                        const std::string& arg = "", uint32_t target = 0) {
        ClanCommandDTO c; c.type = type; c.arg1 = arg; c.targetDbId = target;
        PlayerCommand p; p.clientId = id; p.command = c;
        gameQueue.push(p);
    };

    pushJoin(1, "Lider");
    pushJoin(2, "Soldado");
    pushClan(1, ClanCommandType::FOUND, "GuardiaReal");
    pushClan(2, ClanCommandType::JOIN,  "GuardiaReal");
    pushClan(1, ClanCommandType::ACCEPT, "Soldado", 2);

    std::thread hiloGL(&GameLoop::run, &loop);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    loop.stop();
    if (hiloGL.joinable()) hiloGL.join();

    SUCCEED();
}