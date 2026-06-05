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

uint8_t Protocol::recv_opcode() { return recv_uint8(); }

// =======================================================
// ACTUALIZACIONES DE ESTADO (SERVIDOR -> CLIENTE)
// =======================================================

void Protocol::send_snapshot(const SnapshotDTO& snap) {
    send_uint8(static_cast<uint8_t>(OPCODE::SNAPSHOT));

    send_uint16(static_cast<uint16_t>(snap.players.size()));
    for (const auto& entity: snap.players) {
        send_uint32(entity.id);
        send_uint8(static_cast<uint8_t>(entity.type));
        send_uint16(entity.x);
        send_uint16(entity.y);
        send_uint16(entity.current_hp);
        send_uint16(entity.max_hp);
        send_uint16(entity.sprite_id);
    }

    send_uint16(static_cast<uint16_t>(snap.monsters.size()));
    for (const auto& entity: snap.monsters) {
        send_uint32(entity.id);
        send_uint8(static_cast<uint8_t>(entity.type));
        send_uint16(entity.x);
        send_uint16(entity.y);
        send_uint16(entity.current_hp);
        send_uint16(entity.max_hp);
        send_uint16(entity.sprite_id);
    }

    send_uint16(static_cast<uint16_t>(snap.groundItems.size()));
    for (const auto& item: snap.groundItems) {
        send_uint32(item.itemId);
        send_uint16(item.amount);
        send_uint16(item.x);
        send_uint16(item.y);
    }
}

void Protocol::send_login_success(uint32_t clientId) {
    send_uint8(static_cast<uint8_t>(OPCODE::LOGIN_SUCCESS));
    send_uint32(clientId);
}

void Protocol::send_login_failed(const std::string& errorMessage) {
    send_uint8(static_cast<uint8_t>(OPCODE::LOGIN_FAILED));
    send_string(errorMessage);
}

LoginResponseDTO Protocol::recv_login_response() {
    uint8_t opcode_raw = recv_uint8();
    OPCODE opcode = static_cast<OPCODE>(opcode_raw);

    if (opcode == OPCODE::LOGIN_SUCCESS) {
        uint32_t clientId = recv_uint32();
        return LoginResponseDTO{true, clientId, ""};
    } else if (opcode == OPCODE::LOGIN_FAILED) {
        std::string errorMessage = recv_string();
        return LoginResponseDTO{false, 0, errorMessage};
    } else {
        throw std::runtime_error("Unexpected opcode received when expecting login response");
    }
}

void Protocol::send_register(const RegisterDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::REGISTER));
    send_string(dto.username);
    send_string(dto.password);
}

void Protocol::send_register_success(uint32_t clientId) {
    send_uint8(static_cast<uint8_t>(OPCODE::REGISTER_SUCCESS));
    send_uint32(clientId);
}

void Protocol::send_register_failed(const std::string& errorMessage) {
    send_uint8(static_cast<uint8_t>(OPCODE::REGISTER_FAILED));
    send_string(errorMessage);
}

LoginResponseDTO Protocol::recv_register_response() {
    uint8_t opcode_raw = recv_uint8();
    OPCODE opcode = static_cast<OPCODE>(opcode_raw);

    if (opcode == OPCODE::REGISTER_SUCCESS) {
        uint32_t clientId = recv_uint32();
        return LoginResponseDTO{true, clientId, ""};
    } else if (opcode == OPCODE::REGISTER_FAILED) {
        std::string errorMessage = recv_string();
        return LoginResponseDTO{false, 0, errorMessage};
    } else {
        throw std::runtime_error("Unexpected opcode received when expecting register response");
    }
}

SnapshotDTO Protocol::receive_snapshot_body() {
    SnapshotDTO snap;

    uint16_t players_count = recv_uint16();
    for (uint16_t i = 0; i < players_count; ++i) {
        EntityDTO entity;
        entity.id = recv_uint32();
        entity.type = static_cast<EntityType>(recv_uint8());
        entity.x = recv_uint16();
        entity.y = recv_uint16();
        entity.current_hp = recv_uint16();
        entity.max_hp = recv_uint16();
        entity.sprite_id = recv_uint16();
        snap.players.push_back(entity);
    }

    uint16_t monsters_count = recv_uint16();
    for (uint16_t i = 0; i < monsters_count; ++i) {
        EntityDTO entity;
        entity.id = recv_uint32();
        entity.type = static_cast<EntityType>(recv_uint8());
        entity.x = recv_uint16();
        entity.y = recv_uint16();
        entity.current_hp = recv_uint16();
        entity.max_hp = recv_uint16();
        entity.sprite_id = recv_uint16();
        snap.monsters.push_back(entity);
    }

    uint16_t items_count = recv_uint16();
    for (uint16_t i = 0; i < items_count; ++i) {
        GroundItemDTO item;
        item.itemId = recv_uint32();
        item.amount = recv_uint16();
        item.x = recv_uint16();
        item.y = recv_uint16();
        snap.groundItems.push_back(item);
    }

    return snap;
}

ChatDTO Protocol::receive_chat_body() {
    ChatDTO chat;
    chat.message = recv_string();
    return chat;
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

void Protocol::send_attack(uint32_t targetId) {
    send_uint8(static_cast<uint8_t>(OPCODE::ATTACK));
    send_uint32(targetId);
}

void Protocol::send_drop_item(const DropItemDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::DROP_ITEM));
    send_uint8(dto.slot);
    send_uint16(dto.amount);
}

void Protocol::send_stop_move() { send_uint8(static_cast<uint8_t>(OPCODE::STOP_MOVE)); }

void Protocol::send_equip_item(const EquipItemDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::EQUIP_ITEM));
    send_uint8(dto.slot);
}

void Protocol::send_use_item(const UseItemDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::USE_ITEM));
    send_uint8(dto.slot);
}

void Protocol::send_grab_item() { send_uint8(static_cast<uint8_t>(OPCODE::GRAB_ITEM)); }

void Protocol::send_chat(const ChatDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::CHAT));
    send_string(dto.message);
}

// =======================================================
// LOGICA DE CHAT PRIVADO (CLIENTE -> SERVIDOR)
// =======================================================
void Protocol::send_private_chat(const PrivateChatDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::PRIVATE_CHAT));
    send_string(dto.recipientNick);
    send_string(dto.message);
}

PrivateChatDTO Protocol::receive_private_chat_body() {
    PrivateChatDTO dto;
    dto.recipientNick = recv_string();
    dto.message = recv_string();
    return dto;
}

void Protocol::send_meditate() { send_uint8(static_cast<uint8_t>(OPCODE::MEDITATE)); }

void Protocol::receive_meditate_body() {
    // No hay payload para meditar
}

void Protocol::send_resurrect() { send_uint8(static_cast<uint8_t>(OPCODE::RESURRECT)); }

void Protocol::receive_resurrect_body() {
    // No hay payload para resucitar
}

void Protocol::send_npc_command(const NpcCommandDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::NPC_CMD));
    send_uint8(static_cast<uint8_t>(dto.type));
    send_string(dto.arg);
}

NpcCommandDTO Protocol::receive_npc_command_body() {
    NpcCommandDTO dto;
    dto.type = static_cast<NpcCommandType>(recv_uint8());
    dto.arg = recv_string();
    return dto;
}

void Protocol::send_clan_command(const ClanCommandDTO& dto) {
    send_uint8(static_cast<uint8_t>(OPCODE::CLAN_CMD));
    send_uint8(static_cast<uint8_t>(dto.type));
    send_string(dto.arg1);
    send_uint32(dto.targetDbId);
}

ClanCommandDTO Protocol::receive_clan_command_body() {
    ClanCommandDTO dto;
    dto.type = static_cast<ClanCommandType>(recv_uint8());
    dto.arg1 = recv_string();
    dto.targetDbId = recv_uint32();
    return dto;
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
        case OPCODE::REGISTER: {
            RegisterDTO dto;
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
            AttackDTO dto;
            dto.targetId = recv_uint32();
            return dto;
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
        case OPCODE::PRIVATE_CHAT: {
            return receive_private_chat_body();
        }
        case OPCODE::MEDITATE: {
            receive_meditate_body();
            return MeditateDTO{};
        }
        case OPCODE::RESURRECT: {
            receive_resurrect_body();
            return ResurrectDTO{};
        }
        case OPCODE::NPC_CMD: {
            return receive_npc_command_body();
        }
        case OPCODE::CLAN_CMD: {
            return receive_clan_command_body();
        }
        default:
            throw std::runtime_error("Unknown command received in-game");
    }
}
