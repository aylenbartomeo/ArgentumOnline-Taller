#include "Receiver.h"

#include "../core/Client.h"

Receiver::Receiver(Protocol& protocol, Client& client): protocol(protocol), client(client) {}

void Receiver::run() {
    try {
        while (should_keep_running()) {
            uint8_t opcode = protocol.recv_opcode();
            if (opcode == static_cast<uint8_t>(OPCODE::SNAPSHOT)) {
                SnapshotDTO snap = protocol.receiveSnapshotBody();
                client.pushSnapshot(snap);
            } else if (opcode == static_cast<uint8_t>(OPCODE::CHAT)) {
                ChatDTO chat = protocol.receiveChatBody();
                client.pushChatMessage(chat);
            } else if (opcode == static_cast<uint8_t>(OPCODE::STATS_UPDATE)) {
                PlayerStatsDTO stats = protocol.receivePlayerStatsBody();
                client.pushPlayerStats(stats);
            }
        }
    } catch (...) {}
}
