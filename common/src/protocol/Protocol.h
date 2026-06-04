#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>

#include "../../include/OpCodes.h"
#include "../../include/dto/ClientCommands.h"
#include "../../include/dto/LoginDTO.h"
#include "../../include/dto/LoginResponseDTO.h"
#include "../../include/dto/RegisterDTO.h"
#include "../../include/dto/StartMoveDTO.h"
#include "../socket/socket.h"
#include "dto/Snapshot.h"

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


    CommandVariant receive_command();

    uint8_t recv_opcode();

    // --- MÉTODOS DE ENVÍO (Cliente -> Servidor) ---
    void send_register(const RegisterDTO& dto);
    void send_login(const LoginDTO& loginDTO);
    void send_start_move(const StartMoveDTO& dto);
    void send_stop_move();
    void send_attack();
    void send_drop_item(const DropItemDTO& dto);
    void send_equip_item(const EquipItemDTO& dto);
    void send_use_item(const UseItemDTO& dto);
    void send_grab_item();
    void send_chat(const ChatDTO& dto);
    void send_private_chat(const PrivateChatDTO& dto);
    void send_meditate();
    void send_resurrect();
    void send_npc_command(const NpcCommandDTO& dto);
    void send_clan_command(const ClanCommandDTO& dto);

    // --- MÉTODOS DE ENVÍO (Servidor -> Cliente) ---
    void send_snapshot(const SnapshotDTO& snap);
    SnapshotDTO receive_snapshot_body();

    ChatDTO receive_chat_body();
    PrivateChatDTO receive_private_chat_body();
    void receive_meditate_body();
    void receive_resurrect_body();
    NpcCommandDTO receive_npc_command_body();
    ClanCommandDTO receive_clan_command_body();

    void send_register_success(uint32_t clientId);
    void send_register_failed(const std::string& errorMessage);
    LoginResponseDTO recv_register_response();
    void send_login_success(uint32_t clientId);
    void send_login_failed(const std::string& errorMessage);
    LoginResponseDTO recv_login_response();

    // --- MÉTODO DE ENVÍO DE CHEATS ---
    void send_cheat(const CheatDTO& dto);
};

#endif
