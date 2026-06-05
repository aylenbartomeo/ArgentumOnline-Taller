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

void Protocol::sendUint8(uint8_t value) { skt.sendall(&value, sizeof(uint8_t)); }

void Protocol::sendUint16(uint16_t value) {
    uint16_t net_value = htons(value);

    skt.sendall(reinterpret_cast<const uint8_t*>(&net_value), sizeof(uint16_t));
}

void Protocol::sendUint32(uint32_t value) {
    uint32_t net_value = htonl(value);

    skt.sendall(reinterpret_cast<const uint8_t*>(&net_value), sizeof(uint32_t));
}

void Protocol::sendString(const std::string& str) {
    uint16_t length = htons(static_cast<uint16_t>(str.size()));

    skt.sendall(reinterpret_cast<const uint8_t*>(&length), sizeof(uint16_t));
    skt.sendall(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

uint8_t Protocol::recvUint8() {
    uint8_t value;
    skt.recvall(&value, sizeof(value));

    return value;
}

uint16_t Protocol::recvUint16() {
    uint16_t net_value;
    skt.recvall(&net_value, sizeof(net_value));

    return ntohs(net_value);
}

uint32_t Protocol::recvUint32() {
    uint32_t net_value;
    skt.recvall(&net_value, sizeof(net_value));

    return ntohl(net_value);
}

std::string Protocol::recvString() {
    uint16_t len = recvUint16();
    std::vector<char> buffer(len);

    skt.recvall(buffer.data(), len);

    return std::string(buffer.begin(), buffer.end());
}

uint8_t Protocol::recv_opcode() { return recvUint8(); }

// =======================================================
// ACTUALIZACIONES DE ESTADO (SERVIDOR -> CLIENTE)
// =======================================================

void Protocol::sendSnapshot(const SnapshotDTO& snap) {
    sendUint8(static_cast<uint8_t>(OPCODE::SNAPSHOT));

    sendUint16(static_cast<uint16_t>(snap.players.size()));
    for (const auto& entity: snap.players) {
        sendUint32(entity.id);
        sendUint8(static_cast<uint8_t>(entity.type));
        sendUint16(entity.x);
        sendUint16(entity.y);
        sendUint16(entity.current_hp);
        sendUint16(entity.max_hp);
        sendUint16(entity.sprite_id);
    }

    sendUint16(static_cast<uint16_t>(snap.monsters.size()));
    for (const auto& entity: snap.monsters) {
        sendUint32(entity.id);
        sendUint8(static_cast<uint8_t>(entity.type));
        sendUint16(entity.x);
        sendUint16(entity.y);
        sendUint16(entity.current_hp);
        sendUint16(entity.max_hp);
        sendUint16(entity.sprite_id);
    }

    sendUint16(static_cast<uint16_t>(snap.groundItems.size()));
    for (const auto& item: snap.groundItems) {
        sendUint32(item.itemId);
        sendUint16(item.amount);
        sendUint16(item.x);
        sendUint16(item.y);
    }
}

void Protocol::sendLoginSuccess(uint32_t clientId) {
    sendUint8(static_cast<uint8_t>(OPCODE::LOGIN_SUCCESS));
    sendUint32(clientId);
}

void Protocol::sendLoginFailed(const std::string& errorMessage) {
    sendUint8(static_cast<uint8_t>(OPCODE::LOGIN_FAILED));
    sendString(errorMessage);
}

LoginResponseDTO Protocol::recvLoginResponse() {
    uint8_t opcodeRaw = recvUint8();
    OPCODE opcode = static_cast<OPCODE>(opcodeRaw);

    if (opcode == OPCODE::LOGIN_SUCCESS) {
        uint32_t clientId = recvUint32();
        return LoginResponseDTO{true, clientId, ""};
    } else if (opcode == OPCODE::LOGIN_FAILED) {
        std::string errorMessage = recvString();
        return LoginResponseDTO{false, 0, errorMessage};
    } else {
        throw std::runtime_error("Unexpected opcode received when expecting login response");
    }
}

void Protocol::sendRegister(const RegisterDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::REGISTER));
    sendString(dto.username);
    sendString(dto.password);
}

void Protocol::sendRegisterSuccess(uint32_t clientId) {
    sendUint8(static_cast<uint8_t>(OPCODE::REGISTER_SUCCESS));
    sendUint32(clientId);
}

void Protocol::sendRegisterFailed(const std::string& errorMessage) {
    sendUint8(static_cast<uint8_t>(OPCODE::REGISTER_FAILED));
    sendString(errorMessage);
}

LoginResponseDTO Protocol::recvRegisterResponse() {
    uint8_t opcodeRaw = recvUint8();
    OPCODE opcode = static_cast<OPCODE>(opcodeRaw);

    if (opcode == OPCODE::REGISTER_SUCCESS) {
        uint32_t clientId = recvUint32();
        return LoginResponseDTO{true, clientId, ""};
    } else if (opcode == OPCODE::REGISTER_FAILED) {
        std::string errorMessage = recvString();
        return LoginResponseDTO{false, 0, errorMessage};
    } else {
        throw std::runtime_error("Unexpected opcode received when expecting register response");
    }
}

SnapshotDTO Protocol::receiveSnapshotBody() {
    SnapshotDTO snap;

    uint16_t players_count = recvUint16();
    for (uint16_t i = 0; i < players_count; ++i) {
        EntityDTO entity;
        entity.id = recvUint32();
        entity.type = static_cast<EntityType>(recvUint8());
        entity.x = recvUint16();
        entity.y = recvUint16();
        entity.current_hp = recvUint16();
        entity.max_hp = recvUint16();
        entity.sprite_id = recvUint16();
        snap.players.push_back(entity);
    }

    uint16_t monsters_count = recvUint16();
    for (uint16_t i = 0; i < monsters_count; ++i) {
        EntityDTO entity;
        entity.id = recvUint32();
        entity.type = static_cast<EntityType>(recvUint8());
        entity.x = recvUint16();
        entity.y = recvUint16();
        entity.current_hp = recvUint16();
        entity.max_hp = recvUint16();
        entity.sprite_id = recvUint16();
        snap.monsters.push_back(entity);
    }

    uint16_t items_count = recvUint16();
    for (uint16_t i = 0; i < items_count; ++i) {
        GroundItemDTO item;
        item.itemId = recvUint32();
        item.amount = recvUint16();
        item.x = recvUint16();
        item.y = recvUint16();
        snap.groundItems.push_back(item);
    }

    return snap;
}

ChatDTO Protocol::receiveChatBody() {
    ChatDTO chat;
    chat.message = recvString();
    return chat;
}

// =======================================================
// CAPA SEMÁNTICA (ENVÍO DEL CLIENTE)
// =======================================================

void Protocol::sendLogin(const LoginDTO& loginDTO) {
    sendUint8(static_cast<uint8_t>(OPCODE::LOGIN));
    sendString(loginDTO.username);
    sendString(loginDTO.password);
}

void Protocol::sendStartMove(const StartMoveDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::START_MOVE));
    sendUint8(static_cast<uint8_t>(dto.direction));
}

void Protocol::sendAttack(uint32_t targetId) {
    sendUint8(static_cast<uint8_t>(OPCODE::ATTACK));
    sendUint32(targetId);
}

void Protocol::sendDropItem(const DropItemDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::DROP_ITEM));
    sendUint8(dto.slot);
    sendUint16(dto.amount);
}

void Protocol::sendStopMove() { sendUint8(static_cast<uint8_t>(OPCODE::STOP_MOVE)); }

void Protocol::sendEquipItem(const EquipItemDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::EQUIP_ITEM));
    sendUint8(dto.slot);
}

void Protocol::sendUseItem(const UseItemDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::USE_ITEM));
    sendUint8(dto.slot);
}

void Protocol::sendGrabItem() { sendUint8(static_cast<uint8_t>(OPCODE::GRAB_ITEM)); }

void Protocol::sendChat(const ChatDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::CHAT));
    sendString(dto.message);
}

// =======================================================
// LOGICA DE CHAT PRIVADO (CLIENTE -> SERVIDOR)
// =======================================================
void Protocol::sendPrivateChat(const PrivateChatDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::PRIVATE_CHAT));
    sendString(dto.recipientNick);
    sendString(dto.message);
}

PrivateChatDTO Protocol::receivePrivateChatBody() {
    PrivateChatDTO dto;
    dto.recipientNick = recvString();
    dto.message = recvString();
    return dto;
}

void Protocol::sendMeditate() { sendUint8(static_cast<uint8_t>(OPCODE::MEDITATE)); }

void Protocol::receiveMeditateBody() {
    // No hay payload para meditar
}

void Protocol::sendResurrect() { sendUint8(static_cast<uint8_t>(OPCODE::RESURRECT)); }

void Protocol::receiveResurrectBody() {
    // No hay payload para resucitar
}

void Protocol::sendNpcCommand(const NpcCommandDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::NPC_CMD));
    sendUint8(static_cast<uint8_t>(dto.type));
    sendString(dto.arg);
}

NpcCommandDTO Protocol::receiveNpcCommandBody() {
    NpcCommandDTO dto;
    dto.type = static_cast<NpcCommandType>(recvUint8());
    dto.arg = recvString();
    return dto;
}

void Protocol::sendClanCommand(const ClanCommandDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::CLAN_CMD));
    sendUint8(static_cast<uint8_t>(dto.type));
    sendString(dto.arg1);
    sendUint32(dto.targetDbId);
}

ClanCommandDTO Protocol::receiveClanCommandBody() {
    ClanCommandDTO dto;
    dto.type = static_cast<ClanCommandType>(recvUint8());
    dto.arg1 = recvString();
    dto.targetDbId = recvUint32();
    return dto;
}

// =======================================================
// CAPA SEMÁNTICA (ENVÍO DE CHEATS)
// =======================================================
void Protocol::sendCheat(const CheatDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::OPCODE_CHEAT));
    sendUint8(static_cast<uint8_t>(dto.type));
}

// =======================================================
// CAPA SEMÁNTICA (RECEPCIÓN DEL SERVIDOR)
// =======================================================

CommandVariant Protocol::receive_command() {
    uint8_t opcodeRaw = recvUint8();
    OPCODE opcode = static_cast<OPCODE>(opcodeRaw);

    switch (opcode) {
        case OPCODE::LOGIN: {
            LoginDTO dto;
            dto.username = recvString();
            dto.password = recvString();
            return dto;
        }
        case OPCODE::REGISTER: {
            RegisterDTO dto;
            dto.username = recvString();
            dto.password = recvString();
            return dto;
        }
        case OPCODE::START_MOVE: {
            StartMoveDTO dto;
            dto.direction = static_cast<Movement>(recvUint8());
            return dto;
        }
        case OPCODE::STOP_MOVE: {
            return StopMoveDTO{};
        }
        case OPCODE::ATTACK: {
            AttackDTO dto;
            dto.targetId = recvUint32();
            return dto;
        }
        case OPCODE::DROP_ITEM: {
            DropItemDTO dto;
            dto.slot = recvUint8();
            dto.amount = recvUint16();
            return dto;
        }
        case OPCODE::EQUIP_ITEM: {
            EquipItemDTO dto;
            dto.slot = recvUint8();
            return dto;
        }
        case OPCODE::GRAB_ITEM: {
            return GrabItemDTO{};
        }
        case OPCODE::CHAT: {
            ChatDTO dto;
            dto.message = recvString();
            return dto;
        }
        case OPCODE::USE_ITEM: {
            UseItemDTO dto;
            dto.slot = recvUint8();
            return dto;
        }
        case OPCODE::PRIVATE_CHAT: {
            return receivePrivateChatBody();
        }
        case OPCODE::MEDITATE: {
            receiveMeditateBody();
            return MeditateDTO{};
        }
        case OPCODE::RESURRECT: {
            receiveResurrectBody();
            return ResurrectDTO{};
        }
        case OPCODE::NPC_CMD: {
            return receiveNpcCommandBody();
        }
        case OPCODE::CLAN_CMD: {
            return receiveClanCommandBody();
        }
        case OPCODE::OPCODE_CHEAT: {
            CheatDTO dto;
            dto.type = static_cast<CheatType>(recvUint8());
            return dto;
        }
        default:
            throw std::runtime_error("Unknown command received in-game");
    }
}
