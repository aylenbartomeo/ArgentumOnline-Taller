#include <gtest/gtest.h>
#include <variant>

#include "../common/include/queue.h"
#include "../common/src/Snapshot.h"
#include "../common/src/CommandDTO.h"
#include "../common/include/dto/ClientCommands.h" 
#include "../server/include/model/ServerEvents.h" 
#include "../server/src/ConnectionMonitor.h"

// =======================================================================
// TESTS DE LA COLA CENTRAL (GameEvent y std::variant)
// =======================================================================

TEST(ServerArchitectureTest, Queue_CanPushAndPopJoinEvent) {
    Queue<GameEvent> gameQueue;

    // Simulamos lo que hace el Receiver cuando alguien se loguea
    JoinEvent joinEvent;
    joinEvent.clientId = 100;
    joinEvent.username = "Franco";

    gameQueue.push(joinEvent);

    // Simulamos lo que hace el GameLoop al procesar
    GameEvent poppedEvent = gameQueue.pop();

    // Verificamos que el paquete sea efectivamente un JoinEvent
    ASSERT_TRUE(std::holds_alternative<JoinEvent>(poppedEvent));

    // Extraemos y validamos los datos
    JoinEvent receivedJoin = std::get<JoinEvent>(poppedEvent);
    EXPECT_EQ(receivedJoin.clientId, (uint32_t)100);
    EXPECT_EQ(receivedJoin.username, "Franco");
}

TEST(ServerArchitectureTest, Queue_CanPushAndPopPlayerCommand) {
    Queue<GameEvent> gameQueue;

    // Simulamos lo que hace el Receiver en medio del juego usando la nueva arquitectura
    StartMoveDTO moveDto;
    moveDto.direction = static_cast<uint8_t>(Movement::UP);
    
    // Envolvemos el movimiento en un PlayerCommand indicando quién lo envió
    PlayerCommand pCmd;
    pCmd.clientId = 50;
    pCmd.command = moveDto;

    gameQueue.push(pCmd);

    // Simulamos el GameLoop
    GameEvent poppedEvent = gameQueue.pop();

    // Verificamos que el paquete sea un PlayerCommand
    ASSERT_TRUE(std::holds_alternative<PlayerCommand>(poppedEvent));

    // Extraemos el PlayerCommand
    PlayerCommand receivedCmd = std::get<PlayerCommand>(poppedEvent);
    EXPECT_EQ(receivedCmd.clientId, (uint32_t)50);

    // Verificamos que adentro del PlayerCommand haya un StartMoveDTO
    ASSERT_TRUE(std::holds_alternative<StartMoveDTO>(receivedCmd.command));
    StartMoveDTO receivedMove = std::get<StartMoveDTO>(receivedCmd.command);
    EXPECT_EQ(receivedMove.direction, static_cast<uint8_t>(Movement::UP));
}

// =======================================================================
// TESTS DEL CONNECTION MONITOR (El Broadcaster)
// =======================================================================

TEST(ServerArchitectureTest, ConnectionMonitor_BroadcastsToMultipleClients) {
    ConnectionMonitor monitor;

    Queue<SnapshotDTO> queuePlayer1;
    Queue<SnapshotDTO> queuePlayer2;
    Queue<SnapshotDTO> queuePlayer3;

    monitor.addClient(1, &queuePlayer1);
    monitor.addClient(2, &queuePlayer2);
    monitor.addClient(3, &queuePlayer3);

    SnapshotDTO snap;
    EntityDTO dummyEntity;
    dummyEntity.id = 999;
    snap.entities.push_back(dummyEntity);

    monitor.broadcast(snap);

    SnapshotDTO received1 = queuePlayer1.pop();
    SnapshotDTO received2 = queuePlayer2.pop();
    SnapshotDTO received3 = queuePlayer3.pop();

    EXPECT_EQ(received1.entities.size(), (size_t)1);
    EXPECT_EQ(received1.entities[0].id, (uint32_t)999);
    EXPECT_EQ(received2.entities.size(), (size_t)1);
    EXPECT_EQ(received3.entities.size(), (size_t)1);
}

TEST(ServerArchitectureTest, ConnectionMonitor_IgnoresRemovedClients) {
    ConnectionMonitor monitor;
    Queue<SnapshotDTO> queuePlayer1;
    Queue<SnapshotDTO> queuePlayer2;

    monitor.addClient(1, &queuePlayer1);
    monitor.addClient(2, &queuePlayer2);

    // El jugador 2 se desconecta
    monitor.removeClient(2);

    SnapshotDTO snap;
    monitor.broadcast(snap);

    // El jugador 1 recibe el snapshot
    SnapshotDTO received1;
    bool success = queuePlayer1.try_pop(received1);
    EXPECT_TRUE(success);

    // La cola del jugador 2 DEBE estar vacía
    SnapshotDTO received2;
    bool success2 = queuePlayer2.try_pop(received2);
    EXPECT_FALSE(success2);
}