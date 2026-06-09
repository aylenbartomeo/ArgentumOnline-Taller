#ifndef CLIENT_COMMANDS_H
#define CLIENT_COMMANDS_H

#include <cstdint>
#include <string>
#include <variant>

#include "CheatDTO.h"
#include "ClanCommandDTO.h"
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

struct MeditateDTO {};

struct ResurrectDTO {};

struct ChatDTO {
    std::string message;
};

struct PrivateChatDTO {
    std::string recipientNick;
    std::string message;
};

struct SelectNpcDTO {
    uint32_t npcId;
};

struct ShootDTO {
    float targetX;
    float targetY;
};

enum NpcCommandType { RESPAWN, HEAL, BUY, SELL, DEPOSIT, WITHDRAW, LIST };

struct NpcCommandDTO {
    NpcCommandType type;
    std::string arg;
};

// Esto es lo que la cola del Servidor va a recibir.
using CommandVariant =
        std::variant<RegisterDTO, LoginDTO, StartMoveDTO, StopMoveDTO, AttackDTO, UseItemDTO,
                     EquipItemDTO, DropItemDTO, GrabItemDTO, ChatDTO, PrivateChatDTO, SelectNpcDTO,
                     NpcCommandDTO, ClanCommandDTO, MeditateDTO, ResurrectDTO, CheatDTO, ShootDTO>;

#endif  // CLIENT_COMMANDS_H
