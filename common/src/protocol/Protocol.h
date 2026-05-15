#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "../../include/dto/LoginDTO.h"
#include "../../include/dto/StartMoveDTO.h"
#include "../../include/dto/AttackDTO.h"
#include "../socket/socket.h"


enum class OPCODE : uint8_t {
    LOGIN = 0x01,
    START_MOVE = 0x02,
    STOP_MOVE = 0x03,
    ATTACK = 0x04
};

class Protocol {
public:
    static void send_login(const LoginDTO& loginDTO, Socket& skt);
    static LoginDTO receive_login(Socket& skt);

    static void send_start_move(const StartMoveDTO& startMoveDTO, Socket& skt);
    static StartMoveDTO receive_start_move(Socket& skt);

    static void send_attack(const AttackDTO& attackDTO, Socket& skt);
    static AttackDTO receive_attack(Socket& skt);
};


#endif