#ifndef SERVER_EVENTS_H
#define SERVER_EVENTS_H

#include <string>
#include <variant>
#include "../../common/include/dto/ClientCommands.h"

// Evento cuando alguien se loguea
struct JoinEvent {
    uint32_t clientId;
    std::string username;
};

// Evento que "envuelve" cualquier comando in-game con el ID del jugador
struct PlayerCommand {
    uint32_t clientId;
    CommandVariant command; // Aquí viaja el StartMoveDTO, DropItemDTO, etc.
};

// Evento de desconexión
struct DisconnectEvent {
    uint32_t clientId;
};

// Lo que lee el GameLoop
using GameEvent = std::variant<JoinEvent, PlayerCommand, DisconnectEvent>;

#endif // SERVER_EVENTS_H