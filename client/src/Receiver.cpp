#include "Receiver.h"

#include "Client.h"

Receiver::Receiver(Protocol& protocol, Client& client): protocol(protocol), client(client) {}

void Receiver::run() {
    try {
        while (should_keep_running()) {
            uint8_t opcode = protocol.recv_opcode();
            if (opcode == static_cast<uint8_t>(OPCODE::SNAPSHOT)) {
                SnapshotDTO snap = protocol.receive_snapshot_body();
                client.pushSnapshot(snap);
            } else if (opcode == static_cast<uint8_t>(OPCODE::CHAT)) {
                ChatDTO chat = protocol.receive_chat_body();
                client.pushChatMessage(chat);
            }
        }
    } catch (...) {}
}
