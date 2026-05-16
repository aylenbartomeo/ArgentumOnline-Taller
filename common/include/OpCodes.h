#ifndef OPCODES_H
#define OPCODES_H

#include <cstdint>

enum class OPCODE : uint8_t {
    // Client to Server
    LOGIN = 0x01,
    START_MOVE = 0x02,
    STOP_MOVE = 0x03,
    ATTACK = 0x04,
    SNAPSHOT = 0x05

    // Server to Client
};


#endif
