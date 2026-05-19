#include "Receiver.h"

#include "Client.h"

Receiver::Receiver(Protocol& protocol, Client& client): protocol(protocol), client(client) {}

void Receiver::run() {
    try {
        while (should_keep_running()) {
            SnapshotDTO snap = protocol.receive_snapshot();
            client.pushSnapshot(snap);
        }
    } catch (...) {}
}
