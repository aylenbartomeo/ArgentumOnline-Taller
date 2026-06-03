#ifndef RECEIVER_H
#define RECEIVER_H

#include <atomic>
#include <string>

#include "../../common/include/dto/ServerMessage.h"
#include "../../common/include/queue.h"
#include "../../common/include/thread.h"
#include "../../common/src/protocol/Protocol.h"
#include "../../common/src/socket/socket.h"
#include "../include/ServerEvents.h"
#include "auth/AuthManager.h"
#include "dto/CommandDTO.h"
#include "dto/Snapshot.h"

#include "ConnectionMonitor.h"


/**
 * @class Receiver
 * @brief Hilo encargado de la recepción de mensajes desde el cliente hacia el servidor.
 * * Deserializa las acciones entrantes mediante el protocolo y las deposita en una cola
 * compartida para su procesamiento posterior.
 */
class Receiver: public Thread {
private:
    std::string username;
    Socket& skt;
    uint32_t clientId;
    Queue<GameEvent>& gameQueue;
    Protocol protocolo;
    ConnectionMonitor& monitor;
    AuthManager& auth;
    Queue<ServerMessageVariant>& senderQueue;

public:
    explicit Receiver(Socket& skt, Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor,
                      AuthManager& auth, Queue<ServerMessageVariant>& senderQueue);

    bool authenticatePlayer();
    void inGameCommunication();
    uint32_t getClientId() const;

    void run() override;

    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    Receiver(Receiver&&) = default;
    Receiver& operator=(Receiver&&) = default;
};

#endif
