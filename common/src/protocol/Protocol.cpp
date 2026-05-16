#include "Protocol.h"

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <arpa/inet.h>

Protocol::Protocol(Socket& skt): skt(skt) {}

// =======================================================
// CAPA DE BAJO NIVEL
// =======================================================

void Protocol::send_uint8(uint8_t value) { skt.sendall(&value, sizeof(uint8_t)); }

void Protocol::send_uint16(uint16_t value) {
    uint16_t net_value = htons(value);

    skt.sendall(reinterpret_cast<const uint8_t*>(&net_value), sizeof(uint16_t));
}

void Protocol::send_uint32(uint32_t value) {
    uint32_t net_value = htonl(value);

    skt.sendall(reinterpret_cast<const uint8_t*>(&net_value), sizeof(uint32_t));
}

void Protocol::send_string(const std::string& str) {
    uint16_t length = htons(static_cast<uint16_t>(str.size()));

    skt.sendall(reinterpret_cast<const uint8_t*>(&length), sizeof(uint16_t));
    skt.sendall(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

uint8_t Protocol::recv_uint8() {
    uint8_t value;
    skt.recvall(&value, sizeof(value));

    return value;
}

uint16_t Protocol::recv_uint16() {
    uint16_t net_value;
    skt.recvall(&net_value, sizeof(net_value));

    return ntohs(net_value);
}

uint32_t Protocol::recv_uint32() {
    uint32_t net_value;
    skt.recvall(&net_value, sizeof(net_value));

    return ntohl(net_value);
}

std::string Protocol::recv_string() {
    uint16_t len = recv_uint16();
    std::vector<char> buffer(len);

    skt.recvall(buffer.data(), len);

    return std::string(buffer.begin(), buffer.end());
}

// =======================================================
// CAPA SEMÁNTICA
// =======================================================

void Protocol::send_login(const LoginDTO& loginDTO) {
    send_uint8(static_cast<uint8_t>(OPCODE::LOGIN));
    send_string(loginDTO.username);
    send_string(loginDTO.password);
}

/*void Protocol::receive_login(CommandDTO& dto) {
    dto.username = recv_string();
    dto.password = recv_string();
}*/

uint8_t Protocol::receiveAction() { return recv_uint8(); }

std::string Protocol::receiveUsername() { return recv_string(); }

std::string Protocol::receivePassword() { return recv_string(); }

void Protocol::send_start_move(const CommandDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::START_MOVE));
    send_uint8(static_cast<uint8_t>(dto.movement));
}

void Protocol::receive_start_move(CommandDTO& dto) {
    dto.type = ActionType::MOVE;
    dto.movement = static_cast<Movement>(recv_uint8());
}

void Protocol::send_snapshot(const SnapshotDTO& snap) {
    send_uint8(static_cast<uint8_t>(OPCODE::SNAPSHOT));

    send_uint16(static_cast<uint16_t>(snap.entities.size()));

    for (const auto& entity: snap.entities) {
        send_uint32(entity.id);
        send_uint8(static_cast<uint8_t>(entity.type));
        send_uint16(entity.x);
        send_uint16(entity.y);
        send_uint16(entity.current_hp);
        send_uint16(entity.max_hp);
        send_uint16(entity.sprite_id);
    }
}

SnapshotDTO Protocol::receive_snapshot() {
    SnapshotDTO snap;

    uint16_t count = recv_uint16();

    for (uint16_t i = 0; i < count; ++i) {
        EntityDTO entity;

        entity.id = recv_uint32();
        entity.type = static_cast<EntityType>(recv_uint8());
        entity.x = recv_uint16();
        entity.y = recv_uint16();
        entity.current_hp = recv_uint16();
        entity.max_hp = recv_uint16();
        entity.sprite_id = recv_uint16();

        snap.entities.push_back(entity);
    }

    return snap;
}

void Protocol::receive_command(CommandDTO& dto) {
    uint8_t opcode_raw = recv_uint8();
    OPCODE opcode = static_cast<OPCODE>(opcode_raw);

    switch (opcode) {
        case OPCODE::START_MOVE:
            dto.type = ActionType::MOVE;
            receive_start_move(dto);
            break;

        default:
            throw std::runtime_error("Comando desconocido en recepción in-game");
    }
}
