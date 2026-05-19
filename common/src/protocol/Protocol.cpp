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
// ACTUALIZACIONES DE ESTADO (SERVIDOR -> CLIENTE)
// =======================================================

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
    uint8_t opcode = recv_uint8();
    (void)opcode;
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

// =======================================================
// CAPA SEMÁNTICA (ENVÍO DEL CLIENTE)
// =======================================================

void Protocol::send_login(const LoginDTO& loginDTO) {
    send_uint8(static_cast<uint8_t>(OPCODE::LOGIN));
    send_string(loginDTO.username);
    send_string(loginDTO.password);
}

void Protocol::send_start_move(const StartMoveDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::START_MOVE));
    send_uint8(static_cast<uint8_t>(dto.direction));
}

void Protocol::send_attack() {
    send_uint8(static_cast<uint8_t>(OPCODE::ATTACK));
}

void Protocol::send_drop_item(const DropItemDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::DROP_ITEM));
    send_uint8(dto.slot);
    send_uint16(dto.amount);
}

void Protocol::send_stop_move() {
    send_uint8(static_cast<uint8_t>(OPCODE::STOP_MOVE));
}

void Protocol::send_equip_item(const EquipItemDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::EQUIP_ITEM));
    send_uint8(dto.slot);
}

void Protocol::send_use_item(const UseItemDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::USE_ITEM));
    send_uint8(dto.slot);
}

void Protocol::send_grab_item() {
    send_uint8(static_cast<uint8_t>(OPCODE::GRAB_ITEM));
}

void Protocol::send_chat(const ChatDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::CHAT));
    send_string(dto.message);
}

// =======================================================
// CAPA SEMÁNTICA (RECEPCIÓN DEL SERVIDOR)
// =======================================================

CommandVariant Protocol::receive_command() {
    uint8_t opcode_raw = recv_uint8();
    OPCODE opcode = static_cast<OPCODE>(opcode_raw);

    switch (opcode) {
        case OPCODE::LOGIN: {
            LoginDTO dto;
            dto.username = recv_string();
            dto.password = recv_string();
            return dto;
        }
        case OPCODE::START_MOVE: {
            StartMoveDTO dto;
            dto.direction = static_cast<Movement>(recv_uint8());
            return dto;
        }
        case OPCODE::STOP_MOVE: {
            return StopMoveDTO{};
        }
        case OPCODE::ATTACK: {
            return AttackDTO{};
        }
        case OPCODE::DROP_ITEM: {
            DropItemDTO dto;
            dto.slot = recv_uint8();
            dto.amount = recv_uint16();
            return dto;
        }
        case OPCODE::EQUIP_ITEM: {
            EquipItemDTO dto;
            dto.slot = recv_uint8();
            return dto;
        }
        case OPCODE::GRAB_ITEM: {
            return GrabItemDTO{};
        }
        case OPCODE::CHAT: {
            ChatDTO dto;
            dto.message = recv_string();
            return dto;
        }
        case OPCODE::USE_ITEM: {
            UseItemDTO dto;
            dto.slot = recv_uint8();
            return dto;
        }
        default:
            throw std::runtime_error("Unknown command received in-game");
    }
}

