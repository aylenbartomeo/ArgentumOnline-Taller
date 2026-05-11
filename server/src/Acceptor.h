#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <list>
#include <memory>
#include "../common/include/thread.h"
#include "../common/src/socket/socket.h"
#include "ClientHandler.h"

/**
 * @class Acceptor
 * @brief Hilo encargado de aceptar nuevas conexiones entrantes y gestionar el listado de clientes.
 * * Establece la conexión inicial, instancia los ClientHandlers correspondientes y se encarga
 * de la limpieza periódica de recursos de clientes desconectados.
 */
class Acceptor: public Thread {
private:
    Socket skt;
    std::list<ClientHandler*> clients;

    /**
     * @brief Identifica y elimina de la lista a los clientes que han finalizado su conexión.
     */
    void reap();

    /**
     * @brief Fuerza el cierre de todas las conexiones activas y limpia la lista de clientes.
     */
    void close_clients();

public:
    explicit Acceptor(const char* port);

    virtual void run() override;
    virtual void stop() override;

    /* Deshabilitamos las copias */
    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    /* Habilitamos el movimiento */
    Acceptor(Acceptor&&) = default;
    Acceptor& operator=(Acceptor&&) = default;
};

#endif
