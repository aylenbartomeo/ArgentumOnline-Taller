#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "../../include/dto/LoginDTO.h"
#include "../../include/dto/StartMoveDTO.h"
#include "../../include/dto/AttackDTO.h"
#include "../socket/socket.h"
#include "../../include/OpCodes.h"
#include "Snapshot.h"


class Protocol {
private:
    Socket& skt;

    void send_uint8(uint8_t value);
    void send_uint16(uint16_t value);
    void send_uint32(uint32_t value);

    void send_string(const std::string& str);

    uint8_t recv_uint8();
    uint16_t recv_uint16();
    uint32_t recv_uint32();

    std::string recv_string();

public:
    explicit Protocol(Socket& skt);

    void send_login(const LoginDTO& loginDTO);
    LoginDTO receive_login();

    void send_start_move(const StartMoveDTO& dto);
    StartMoveDTO receive_start_move();

    // --- ENVÍO (SERVIDOR -> CLIENTE) ---
    void send_snapshot(const SnapshotDTO& snap);

    // --- RECEPCIÓN (CLIENTE LEYENDO AL SERVIDOR) ---
    SnapshotDTO receive_snapshot();
};

#endif