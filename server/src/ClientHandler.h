#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <atomic>
#include <string>

#include "../../common/include/queue.h"
#include "../../common/include/thread.h"
#include "../../common/src/socket/socket.h"

#include "Receiver.h"
#include "Sender.h"

/**
 * @class ClientHandler
 * @brief Gestiona la conexión individual de un cliente y sus hilos de comunicación.
 * * Coordina el ciclo de vida del Sender y Receiver, administrando sus hilos
 * y la cola de eventos específica para el envío de datos hacia el cliente.
 */
class ClientHandler {
private:
    Socket skt;
    Receiver receiver;
    Sender sender;

public:
    explicit ClientHandler(Socket&& skt);

    void start_threads();
    void stop_threads();
    void join_threads();

    bool is_online() const;


    // Queue<EventDTO>& get_event_queue();

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
};

#endif
