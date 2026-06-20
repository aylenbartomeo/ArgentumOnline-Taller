#include <variant>

#include <gtest/gtest.h>

#include "../common/include/dto/ClientCommands.h"
#include "../common/include/dto/ServerMessage.h"
#include "../common/include/queue.h"
#include "../server/include/ServerEvents.h"
#include "../server/src/ConnectionMonitor.h"
#include "dto/CommandDTO.h"
#include "dto/Snapshot.h"

// =======================================================================
// TESTS DE LA COLA CENTRAL
// =======================================================================

TEST(ServerArchitectureTest, Queue_CanPushAndPopJoinEvent) {
    Queue<GameEvent> gameQueue;

    JoinEvent joinEvent;
    joinEvent.clientId = 100;
    joinEvent.username = "Franco";

    gameQueue.push(joinEvent);

    GameEvent poppedEvent = gameQueue.pop();

    ASSERT_TRUE(std::holds_alternative<JoinEvent>(poppedEvent));

    JoinEvent receivedJoin = std::get<JoinEvent>(poppedEvent);
    EXPECT_EQ(receivedJoin.clientId, (uint32_t)100);
    EXPECT_EQ(receivedJoin.username, "Franco");
}

TEST(ServerArchitectureTest, Queue_CanPushAndPopPlayerCommand) {
    Queue<GameEvent> gameQueue;

    StartMoveDTO moveDto;
    moveDto.direction = Movement::UP;

    PlayerCommand pCmd;
    pCmd.clientId = 50;
    pCmd.command = moveDto;

    gameQueue.push(pCmd);

    // Simulamos el GameLoop
    GameEvent poppedEvent = gameQueue.pop();

    ASSERT_TRUE(std::holds_alternative<PlayerCommand>(poppedEvent));

    PlayerCommand receivedCmd = std::get<PlayerCommand>(poppedEvent);
    EXPECT_EQ(receivedCmd.clientId, (uint32_t)50);

    ASSERT_TRUE(std::holds_alternative<StartMoveDTO>(receivedCmd.command));
    StartMoveDTO receivedMove = std::get<StartMoveDTO>(receivedCmd.command);
    EXPECT_EQ(receivedMove.direction, Movement::UP);
}

// =======================================================================
// TESTS DEL CONNECTION MONITOR
// =======================================================================

TEST(ServerArchitectureTest, ConnectionMonitor_BroadcastsToMultipleClients) {
    ConnectionMonitor monitor;

    Queue<ServerMessageVariant> queuePlayer1;
    Queue<ServerMessageVariant> queuePlayer2;
    Queue<ServerMessageVariant> queuePlayer3;

    monitor.addClient(1, &queuePlayer1);
    monitor.addClient(2, &queuePlayer2);
    monitor.addClient(3, &queuePlayer3);

    SnapshotDTO snap;
    EntityDTO dummyEntity;
    dummyEntity.id = 999;
    snap.players.push_back(dummyEntity);

    monitor.broadcast(snap);

    SnapshotDTO received1 = std::get<SnapshotDTO>(queuePlayer1.pop());
    SnapshotDTO received2 = std::get<SnapshotDTO>(queuePlayer2.pop());
    SnapshotDTO received3 = std::get<SnapshotDTO>(queuePlayer3.pop());

    EXPECT_EQ(received1.players.size(), (size_t)1);
    EXPECT_EQ(received1.players[0].id, (uint32_t)999);
    EXPECT_EQ(received2.players.size(), (size_t)1);
    EXPECT_EQ(received3.players.size(), (size_t)1);
}

TEST(ServerArchitectureTest, ConnectionMonitor_IgnoresRemovedClients) {
    ConnectionMonitor monitor;
    Queue<ServerMessageVariant> queuePlayer1;
    Queue<ServerMessageVariant> queuePlayer2;

    monitor.addClient(1, &queuePlayer1);
    monitor.addClient(2, &queuePlayer2);

    monitor.removeClient(2);

    SnapshotDTO snap;
    monitor.broadcast(snap);

    ServerMessageVariant received1;
    bool success = queuePlayer1.try_pop(received1);
    EXPECT_TRUE(success);

    ServerMessageVariant received2;
    bool success2 = queuePlayer2.try_pop(received2);
    EXPECT_FALSE(success2);
}

TEST(ServerArchitectureTest, ConnectionMonitor_TracksActiveSessions) {
    ConnectionMonitor monitor;
    Queue<ServerMessageVariant> dummyQueue;
    uint32_t playerId = 1;

    EXPECT_FALSE(monitor.isClientConnected(playerId));

    monitor.addClient(playerId, &dummyQueue);

    EXPECT_TRUE(monitor.isClientConnected(playerId));

    monitor.removeClient(playerId);

    EXPECT_FALSE(monitor.isClientConnected(playerId));
}
