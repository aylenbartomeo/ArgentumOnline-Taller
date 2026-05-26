#ifndef CONNECTION_MONITOR_H
#define CONNECTION_MONITOR_H

#include <cstdint>
#include <mutex>
#include <unordered_map>

#include "../../common/include/dto/ServerMessage.h"
#include "../../common/include/queue.h"
#include "dto/Snapshot.h"

/**
 * @class ConnectionMonitor
 * @brief Monitor thread-safe encargado de registrar las colas de salida activas
 * y coordinar la distribución masiva de snapshots (broadcast).
 */
class ConnectionMonitor {
private:
    std::mutex mtx;
    // Mapea el ID único del cliente con el puntero a su respectiva cola del Sender
    std::unordered_map<uint32_t, Queue<ServerMessageVariant>*> clientQueues;

public:
    ConnectionMonitor() = default;

    void addClient(uint32_t clientId, Queue<ServerMessageVariant>* queue);

    void removeClient(uint32_t clientId);

    bool isClientConnected(uint32_t clientId);

    void broadcast(const SnapshotDTO& snapshot);

    void sendToClient(uint32_t clientId, const ChatDTO& msg);

    ConnectionMonitor(const ConnectionMonitor&) = delete;
    ConnectionMonitor& operator=(const ConnectionMonitor&) = delete;

    ConnectionMonitor(ConnectionMonitor&&) = default;
    ConnectionMonitor& operator=(ConnectionMonitor&&) = default;
};

#endif  // CONNECTION_MONITOR_H
