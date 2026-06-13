#ifndef WORLD_PERSIST_DATA_H
#define WORLD_PERSIST_DATA_H

#include <cstdint>

#pragma pack(push, 1)
struct WorldMetadata {
    uint32_t worldId;
    char name[64];
    char baseMapPath[128];
    uint64_t createdAt;
    uint64_t lastSavedAt;
};
#pragma pack(pop)
static_assert(sizeof(WorldMetadata) == 212,
              "WorldMetadata must be 212 bytes");  // 4 + 64 + 128 + 8 + 8 = 212

#pragma pack(push, 1)
struct MonsterPersistData {
    uint32_t entityId;
    uint8_t type;
    uint8_t _pad1[3];
    int32_t posX;
    int32_t posY;
    int16_t hp;
    int16_t maxHp;
};
#pragma pack(pop)
static_assert(sizeof(MonsterPersistData) == 20, "MonsterPersistData must be 20 bytes");

#pragma pack(push, 1)
struct NpcHeaderPersistData {
    uint32_t entityId;
    uint8_t type;
    uint8_t _pad[3];
    int32_t posX;
    int32_t posY;
    uint32_t stockCount;
};
#pragma pack(pop)
static_assert(sizeof(NpcHeaderPersistData) == 20, "NpcHeaderPersistData must be 20 bytes");

#pragma pack(push, 1)
struct NpcStockPersistData {
    uint32_t itemId;
    int32_t amount;
};
#pragma pack(pop)
static_assert(sizeof(NpcStockPersistData) == 8, "NpcStockPersistData must be 8 bytes");

#pragma pack(push, 1)
struct GroundItemPersistData {
    int32_t posX;
    int32_t posY;
    uint32_t itemId;
    uint16_t amount;
    uint8_t _pad[2];
};
#pragma pack(pop)
static_assert(sizeof(GroundItemPersistData) == 16, "GroundItemPersistData must be 16 bytes");

#pragma pack(push, 1)
struct BankSlotPersistData {
    uint32_t itemId;
    uint16_t amount;
    uint8_t _pad[2];
};
#pragma pack(pop)
static_assert(sizeof(BankSlotPersistData) == 8, "BankSlotPersistData must be 8 bytes");

#pragma pack(push, 1)
struct BankAccountHeaderPersistData {
    uint32_t playerDbId;
    uint32_t gold;
    uint32_t slotCount;
};
#pragma pack(pop)
static_assert(sizeof(BankAccountHeaderPersistData) == 12,
              "BankAccountHeaderPersistData must be 12 bytes");

#pragma pack(push, 1)
struct ClanPlayerPersistData {
    uint32_t dbId;
};
#pragma pack(pop)
static_assert(sizeof(ClanPlayerPersistData) == 4, "ClanPlayerPersistData must be 4 bytes");

#pragma pack(push, 1)
struct ClanHeaderPersistData {
    uint32_t clanId;
    uint32_t founderDbId;
    char name[64];
    uint32_t memberCount;
    uint32_t pendingCount;
    uint32_t bannedCount;
};
#pragma pack(pop)
static_assert(sizeof(ClanHeaderPersistData) == 84, "ClanHeaderPersistData must be 84 bytes");

#endif  // WORLD_PERSIST_DATA_H
