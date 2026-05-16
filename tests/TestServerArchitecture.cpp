#include <gtest/gtest.h>

#include "../common/include/queue.h"
#include "../common/src/CommandDTO.h"
#include "../common/src/Snapshot.h"
#include "../server/src/ConnectionMonitor.h"

// =======================================================================
// TESTS DE LA COLA CENTRAL (GameEvent y std::variant)
// =======================================================================

TEST(ServerArchitectureTest, Queue_CanPushAndPopJoinEvent) {
    Queue<GameEvent> gameQueue;

    // Simulamos lo que hace el Receiver cuando alguien se loguea
    JoinEvent joinEvent;
    joinEvent.playerId = 100;
    joinEvent.username = "Franco";

    gameQueue.push(joinEvent);

    // Simulamos lo que hace el GameLoop al procesar
    GameEvent poppedEvent = gameQueue.pop();

    // Verificamos que el paquete sea efectivamente un JoinEvent
    ASSERT_TRUE(std::holds_alternative<JoinEvent>(poppedEvent));

    // Extraemos y validamos los datos
    JoinEvent receivedJoin = std::get<JoinEvent>(poppedEvent);
    EXPECT_EQ(receivedJoin.playerId, (uint32_t)100);
    EXPECT_EQ(receivedJoin.username, "Franco");
}

TEST(ServerArchitectureTest, Queue_CanPushAndPopCommandDTO) {
    Queue<GameEvent> gameQueue;

    // Simulamos lo que hace el Receiver en medio del juego
    CommandDTO moveCmd;
    moveCmd.type = ActionType::MOVE;
    moveCmd.movement = Movement::UP;
    moveCmd.playerId = 50;

    gameQueue.push(moveCmd);

    // Simulamos el GameLoop
    GameEvent poppedEvent = gameQueue.pop();

    // Verificamos que el paquete NO sea un JoinEvent, sino un CommandDTO
    ASSERT_TRUE(std::holds_alternative<CommandDTO>(poppedEvent));

    // Extraemos y validamos
    CommandDTO receivedCmd = std::get<CommandDTO>(poppedEvent);
    EXPECT_EQ(receivedCmd.type, ActionType::MOVE);
    EXPECT_EQ(receivedCmd.movement, Movement::UP);
    EXPECT_EQ(receivedCmd.playerId, (uint32_t)50);
}

// =======================================================================
// TESTS DEL CONNECTION MONITOR (El Broadcaster)
// =======================================================================

TEST(ServerArchitectureTest, ConnectionMonitor_BroadcastsToMultipleClients) {
    ConnectionMonitor monitor;

    // Creamos las colas de 3 jugadores simulados
    Queue<SnapshotDTO> queuePlayer1;
    Queue<SnapshotDTO> queuePlayer2;
    Queue<SnapshotDTO> queuePlayer3;

    // Los conectamos al monitor
    monitor.addClient(1, &queuePlayer1);
    monitor.addClient(2, &queuePlayer2);
    monitor.addClient(3, &queuePlayer3);

    // Creamos un Snapshot simulado
    SnapshotDTO snap;
    EntityDTO dummyEntity;
    dummyEntity.id = 999;
    snap.entities.push_back(dummyEntity);

    // Ejecutamos el broadcast (lo que hace el GameLoop al final de cada frame)
    monitor.broadcast(snap);

    // Verificamos que los 3 jugadores hayan recibido exactamente la misma foto
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

    // El jugador 2 se desconecta y el Acceptor lo saca del monitor
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
