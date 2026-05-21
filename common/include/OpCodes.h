#ifndef OPCODES_H
#define OPCODES_H

#include <cstdint>

enum class OPCODE : uint8_t {
    // --- CLIENTE A SERVIDOR ---
    LOGIN = 0x01,
    REGISTER = 0x0A,
    START_MOVE = 0x02,
    STOP_MOVE = 0x03,
    ATTACK = 0x04,
    USE_ITEM = 0x05,
    EQUIP_ITEM = 0x06,
    DROP_ITEM = 0x07,
    GRAB_ITEM = 0x08,
    CHAT = 0x09,

    // --- SERVIDOR A CLIENTE ---
    SNAPSHOT = 0x82,
    STATS_UPDATE = 0x83,  // Envía el Oro, Maná, Exp, Nivel y el Inventario completo.
    INVENTORY_UPDATE = 0x84,
    LOGIN_SUCCESS = 0x85,
    LOGIN_FAILED = 0x86,
    REGISTER_SUCCESS = 0x87,
    REGISTER_FAILED = 0x88,
};

#endif
