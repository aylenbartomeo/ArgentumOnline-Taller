#ifndef SENDER_H
#define SENDER_H

#include <atomic>
#include <string>
#include "../common/include/queue.h"
#include "../common/src/socket/socket.h"
#include "../common/include/thread.h"   
#include "../common/src/Protocol.h"

/**
 * @class Sender
 * @brief Hilo especializado en el envío de mensajes desde el servidor hacia un cliente.
 * * Consume eventos de una cola sincronizada y los serializa a través del protocolo
 * para su transmisión mediante un socket.
 */
class Sender: public Thread {
private:
    Protocol protocol;

public:
    Sender(Socket& skt);

    void run() override;
    void stop() override;

    /* Deshabilito las copias */
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;

    /* Permito movimiento */
    Sender(Sender&&) = default;
    Sender& operator=(Sender&&) = default;
};

#endif
