#include "ConnectionMonitor.h"

void ConnectionMonitor::addClient(uint32_t clientId, Queue<SnapshotDTO>* queue) {
    std::lock_guard<std::mutex> lock(this->mtx);
    this->clientQueues[clientId] = queue;
}

void ConnectionMonitor::removeClient(uint32_t clientId) {
    std::lock_guard<std::mutex> lock(this->mtx);
    this->clientQueues.erase(clientId);
}

void ConnectionMonitor::broadcast(const SnapshotDTO& snapshot) {
    std::lock_guard<std::mutex> lock(this->mtx);
    for (auto& pair: this->clientQueues) {
        Queue<SnapshotDTO>* queue = pair.second;
        try {
            // Insertamos la copia del snapshot en cada cola activa.
            // Si el buffer de la cola se llena, no bloqueamos el loop principal.
            queue->push(snapshot);
        } catch (...) {
            // Ignoramos excepciones si una cola fue cerrada concurrentemente en este ciclo
        }
    }
}
