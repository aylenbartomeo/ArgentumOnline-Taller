#ifndef RECEIVER_H
#define RECEIVER_H

#include <atomic>
#include <string>

#include "../common/include/queue.h"
#include "../common/src/socket/socket.h"
#include "../common/include/thread.h"
#include "../common/src/Protocol.h"

/**
 * @class Receiver
 * @brief Hilo encargado de la recepción de mensajes desde el cliente hacia el servidor.
 * * Deserializa las acciones entrantes mediante el protocolo y las deposita en una cola
 * compartida para su procesamiento posterior.
 */
class Receiver: public Thread {
private:
    Protocol protocol;
    std::string username;

public:
    Receiver(Socket& skt);

    void run() override;

    /* Deshabilito las copias */
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    /* Permito movimiento */
    Receiver(Receiver&&) = default;
    Receiver& operator=(Receiver&&) = default;
};

#endif
