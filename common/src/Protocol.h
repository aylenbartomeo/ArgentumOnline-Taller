#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <vector>
#include "socket/socket.h"

/**
 * @class Protocol
 * @brief Encargado de la serialización y deserialización de mensajes según el formato definido.
 * * Traduce entre estructuras de datos de alto nivel (DTOs) y flujos de bytes binarios
 * para garantizar la comunicación consistente entre el cliente y el servidor.
 */
class Protocol {
private:
    Socket& skt;

    /* Aca abajo irian metodos para codificar y decodificar las diferentes acciones*/

public:
    Protocol(Socket& skt);

    /* Deshabilitamos la copia y habilitamos movimiento */
    Protocol(const Protocol&) = delete;
    Protocol& operator=(const Protocol&) = delete;
    Protocol(Protocol&&) = default;
    Protocol& operator=(Protocol&&) = default;

    /* Aca irian los metodos para enviar mensajes y recibir mensajes en los sender y receiver */
};

#endif