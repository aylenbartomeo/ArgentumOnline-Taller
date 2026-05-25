#ifndef CLIENT_COMMANDS_H
#define CLIENT_COMMANDS_H

#include <cstdint>
#include <string>
#include <variant>

#include "LoginDTO.h"
#include "RegisterDTO.h"
#include "StartMoveDTO.h"

// DTOs vacíos (solo indican la intención)
struct StopMoveDTO {};

struct AttackDTO {
    uint32_t targetId;
};

struct GrabItemDTO {};

// DTOs con Payload
struct DropItemDTO {
    uint8_t slot;
    uint16_t amount;
};

struct EquipItemDTO {
    uint8_t slot;
};

struct UseItemDTO {
    uint8_t slot;
};

struct ChatDTO {
    std::string message;
};

struct InteractDTO {
    uint32_t targetId;
};

// Esto es lo que la cola del Servidor va a recibir.
using CommandVariant = std::variant<RegisterDTO, LoginDTO, StartMoveDTO, StopMoveDTO, AttackDTO,
                                    UseItemDTO, EquipItemDTO, DropItemDTO, GrabItemDTO, ChatDTO, InteractDTO>;

#endif  // CLIENT_COMMANDS_H
