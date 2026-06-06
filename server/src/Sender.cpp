#include "Sender.h"

#include <iostream>

Sender::Sender(Socket& skt, Queue<ServerMessageVariant>& senderQueue):
        senderQueue(senderQueue), protocol(skt) {}

void Sender::run() {
    try {
        while (should_keep_running()) {
            ServerMessageVariant msg = senderQueue.pop();

            if (std::holds_alternative<SnapshotDTO>(msg)) {
                protocol.sendSnapshot(std::get<SnapshotDTO>(msg));
            } else if (std::holds_alternative<ChatDTO>(msg)) {
                protocol.sendChat(std::get<ChatDTO>(msg));
            }
        }
    } catch (const ClosedQueue& e) {
        // Flujo esperado: El ClientHandler cerró la cola durante el apagado general
        // o porque el cliente se desconectó. El hilo termina limpiamente.
    } catch (const std::exception& e) {
        // Flujo esperado: El socket se rompió mientras intentábamos escribir (ej: el cliente cerró
        // el juego).
    }
}
