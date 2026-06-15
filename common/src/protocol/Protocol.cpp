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

void Protocol::sendFloat(float value) {
    uint32_t net_value;
    std::memcpy(&net_value, &value, sizeof(float));
    net_value = htonl(net_value);

    skt.sendall(reinterpret_cast<const uint8_t*>(&net_value), sizeof(uint32_t));
}

float Protocol::recvFloat() {
    uint32_t net_value;
    skt.recvall(&net_value, sizeof(net_value));
    net_value = ntohl(net_value);

    float value;
    std::memcpy(&value, &net_value, sizeof(float));
    return value;
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
        sendUint8(entity.entityTypeId);
        sendUint8(entity.action);
        sendUint16(entity.weaponItemId);
        sendUint16(entity.helmetItemId);
        sendUint16(entity.shieldItemId);
        sendUint16(entity.bodyArmorItemId);
        sendUint16(entity.level);
        sendUint8(entity.stateId);  // ID de estado del jugador (vivo, fantasma, meditando)
        sendString(entity.name);
    }

    sendUint16(static_cast<uint16_t>(snap.monsters.size()));
    for (const auto& entity: snap.monsters) {
        sendUint32(entity.id);
        sendUint8(static_cast<uint8_t>(entity.type));
        sendUint16(entity.x);
        sendUint16(entity.y);
        sendUint16(entity.current_hp);
        sendUint16(entity.max_hp);
        sendUint8(entity.entityTypeId);
        sendUint8(entity.action);
        sendUint16(entity.weaponItemId);
        sendUint16(entity.helmetItemId);
        sendUint16(entity.shieldItemId);
        sendUint16(entity.bodyArmorItemId);
        sendUint16(entity.level);
    }

    sendUint16(static_cast<uint16_t>(snap.groundItems.size()));
    for (const auto& item: snap.groundItems) {
        sendUint32(item.itemId);
        sendUint16(item.amount);
        sendUint16(item.x);
        sendUint16(item.y);
    }

    sendUint16(static_cast<uint16_t>(snap.projectiles.size()));
    for (const auto& proj: snap.projectiles) {
        sendUint32(proj.id);
        sendFloat(proj.x);
        sendFloat(proj.y);
        sendFloat(proj.velX);
        sendFloat(proj.velY);
        sendUint16(proj.spriteId);
    }
}

void Protocol::sendPlayerStats(const PlayerStatsDTO& stats) {
    sendUint8(static_cast<uint8_t>(OPCODE::STATS_UPDATE));

    sendUint16(stats.currentHp);
    sendUint16(stats.maxHp);
    sendUint16(stats.currentMana);
    sendUint16(stats.maxMana);
    sendUint32(stats.gold);
    sendUint32(stats.exp);
    sendUint16(stats.level);
    sendUint32(stats.expIntoLevel);
    sendUint32(stats.expForLevel);

    sendUint8(static_cast<uint8_t>(stats.race));
    sendUint8(static_cast<uint8_t>(stats.characterClass));
    sendUint32(stats.agilityBuffTimeLeftMs);
    sendUint32(stats.strengthBuffTimeLeftMs);

    sendUint16(static_cast<uint16_t>(stats.inventory.size()));
    for (const auto& item: stats.inventory) {
        sendUint8(item.slot);
        sendUint32(item.itemId);
        sendUint16(item.amount);
        sendUint8(item.isEquipped ? 1 : 0);
        sendString(item.description);
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

void Protocol::sendJoinResponse(const JoinResponseDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::JOIN_RESPONSE));
    sendUint8(dto.needsCreation ? 1 : 0);

    if (dto.needsCreation) {
        sendUint32(dto.baseStrength);
        sendUint32(dto.baseAgility);
        sendUint32(dto.baseIntelligence);
        sendUint32(dto.baseConstitution);

        sendUint16(static_cast<uint16_t>(dto.raceFactors.size()));
        for (float f: dto.raceFactors) {
            sendFloat(f);
        }

        sendUint16(static_cast<uint16_t>(dto.classFactors.size()));
        for (float f: dto.classFactors) {
            sendFloat(f);
        }
    }
}

JoinResponseDTO Protocol::receiveJoinResponseBody() {

    JoinResponseDTO dto;
    dto.needsCreation = (recvUint8() == 1);

    if (dto.needsCreation) {
        dto.baseStrength = recvUint32();
        dto.baseAgility = recvUint32();
        dto.baseIntelligence = recvUint32();
        dto.baseConstitution = recvUint32();

        uint16_t raceFactorsSize = recvUint16();
        for (uint16_t i = 0; i < raceFactorsSize; ++i) {
            dto.raceFactors.push_back(recvFloat());
        }

        uint16_t classFactorsSize = recvUint16();
        for (uint16_t i = 0; i < classFactorsSize; ++i) {
            dto.classFactors.push_back(recvFloat());
        }
    }

    return dto;
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
        entity.entityTypeId = recvUint8();
        entity.action = recvUint8();
        entity.weaponItemId = recvUint16();
        entity.helmetItemId = recvUint16();
        entity.shieldItemId = recvUint16();
        entity.bodyArmorItemId = recvUint16();
        entity.level = recvUint16();
        entity.stateId = recvUint8();  // ID de estado del jugador (vivo, fantasma, meditando)
        entity.name = recvString();
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
        entity.entityTypeId = recvUint8();
        entity.action = recvUint8();
        entity.weaponItemId = recvUint16();
        entity.helmetItemId = recvUint16();
        entity.shieldItemId = recvUint16();
        entity.bodyArmorItemId = recvUint16();
        entity.level = recvUint16();
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

    uint16_t projectiles_count = recvUint16();
    for (uint16_t i = 0; i < projectiles_count; ++i) {
        ProjectileDTO proj;
        proj.id = recvUint32();
        proj.x = recvFloat();
        proj.y = recvFloat();
        proj.velX = recvFloat();
        proj.velY = recvFloat();
        proj.spriteId = recvUint16();
        snap.projectiles.push_back(proj);
    }

    return snap;
}

PlayerStatsDTO Protocol::receivePlayerStatsBody() {
    PlayerStatsDTO stats;
    stats.currentHp = recvUint16();
    stats.maxHp = recvUint16();
    stats.currentMana = recvUint16();
    stats.maxMana = recvUint16();
    stats.gold = recvUint32();
    stats.exp = recvUint32();
    stats.level = recvUint16();
    stats.expIntoLevel = recvUint32();
    stats.expForLevel = recvUint32();

    stats.race = static_cast<Race>(recvUint8());
    stats.characterClass = static_cast<CharacterClass>(recvUint8());
    stats.agilityBuffTimeLeftMs = recvUint32();
    stats.strengthBuffTimeLeftMs = recvUint32();

    uint16_t items_count = recvUint16();
    for (uint16_t i = 0; i < items_count; ++i) {
        InventorySlotDTO item;
        item.slot = recvUint8();
        item.itemId = recvUint32();
        item.amount = recvUint16();
        item.isEquipped = (recvUint8() == 1);
        item.description = recvString();
        stats.inventory.push_back(item);
    }

    return stats;
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
    sendUint32(dto.npcId);
}

NpcCommandDTO Protocol::receiveNpcCommandBody() {
    NpcCommandDTO dto;
    dto.type = static_cast<NpcCommandType>(recvUint8());
    dto.arg = recvString();
    dto.npcId = recvUint32();
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
// CAPA SEMÁNTICA (ENVIO DE SHOT)
// =======================================================

void Protocol::sendShoot(const ShootDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::SHOOT));
    sendFloat(dto.targetX);
    sendFloat(dto.targetY);
}

void Protocol::sendCreateCharacter(const CreateCharacterDTO& dto) {
    sendUint8(static_cast<uint8_t>(OPCODE::CREATE_CHARACTER));
    sendUint8(dto.race);
    sendUint8(dto.characterClass);
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
        case OPCODE::SHOOT: {
            ShootDTO dto;
            dto.targetX = recvFloat();
            dto.targetY = recvFloat();
            return dto;
        }
        case OPCODE::CREATE_CHARACTER: {
            CreateCharacterDTO dto;
            dto.race = recvUint8();
            dto.characterClass = recvUint8();
            return dto;
        }
        default:
            throw std::runtime_error("Unknown command received in-game");
    }
}
