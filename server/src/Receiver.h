#ifndef RECEIVER_H
#define RECEIVER_H

#include <atomic>
#include <string>

#include "../../common/include/queue.h"
#include "../../common/include/thread.h"
#include "dto/CommandDTO.h"
#include "../../common/src/protocol/Protocol.h"
#include "../../common/src/socket/socket.h"
#include "../include/model/ServerEvents.h"

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

public:
    explicit Receiver(Socket& skt, uint32_t clientId, Queue<GameEvent>& gameQueue);

    bool authenticatePlayer();
    void inGameCommunication();

    void run() override;

    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    Receiver(Receiver&&) = default;
    Receiver& operator=(Receiver&&) = default;
};

#endif
