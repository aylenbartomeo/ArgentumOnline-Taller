#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>

#include "../../include/OpCodes.h"
#include "../../include/dto/AttackDTO.h"
#include "../../include/dto/LoginDTO.h"
#include "../../include/dto/StartMoveDTO.h"
#include "../CommandDTO.h"
#include "../socket/socket.h"

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

    // --- FASE LOBBY / HANDSHAKE ---
    void send_login(const LoginDTO& loginDTO);
    // void receive_login(CommandDTO& dto);

    uint8_t receiveAction();
    std::string receiveUsername();
    std::string receivePassword();

    // --- FASE IN-GAME ---
    void send_start_move(const CommandDTO& dto);
    void receive_start_move(CommandDTO& dto);

    // --- RECIBIR COMANDOS DE CLIENTES EN JUEGO ---
    void receive_command(CommandDTO& dto);

    // --- ENVÍO (SERVIDOR -> CLIENTE) ---
    void send_snapshot(const SnapshotDTO& snap);

    // --- RECEPCIÓN (CLIENTE LEYENDO AL SERVIDOR) ---
    SnapshotDTO receive_snapshot();
};

#endif
