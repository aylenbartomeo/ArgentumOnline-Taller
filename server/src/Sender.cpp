#include "Sender.h"

#include <iostream>

Sender::Sender(Socket& skt, Queue<SnapshotDTO>& senderQueue):
        senderQueue(senderQueue), protocol(skt) {}

void Sender::run() {
    try {
        while (should_keep_running()) {
            SnapshotDTO snap = senderQueue.pop();
            protocol.send_snapshot(snap);
        }
    } catch (const ClosedQueue& e) {
        // Flujo esperado: El ClientHandler cerró la cola durante el apagado general
        // o porque el cliente se desconectó. El hilo termina limpiamente.
    } catch (const std::exception& e) {
        // Flujo esperado: El socket se rompió mientras intentábamos escribir (ej: el cliente cerró
        // el juego).
    }
}
