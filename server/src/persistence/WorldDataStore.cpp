#include "WorldDataStore.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

WorldDataStore::WorldDataStore(const std::string& worldsBaseDir): baseDir(worldsBaseDir) {
    if (!fs::exists(baseDir)) {
        fs::create_directories(baseDir);
    }
}

uint32_t WorldDataStore::loadNextId() const {
    std::string idFilePath = baseDir + "next_id.txt";
    if (!fs::exists(idFilePath)) {
        return 1;
    }
    std::ifstream ifs(idFilePath);
    uint32_t id = 1;
    if (ifs >> id) {
        return id;
    }
    return 1;
}

void WorldDataStore::saveNextId(uint32_t nextId) {
    std::string idFilePath = baseDir + "next_id.txt";
    std::ofstream ofs(idFilePath);
    ofs << nextId;
}

std::string WorldDataStore::getWorldDir(uint32_t worldId) const {
    return baseDir + std::to_string(worldId) + "/";
}

uint32_t WorldDataStore::createWorld(const std::string& name, const std::string& baseMapPath) {
    uint32_t worldId = loadNextId();
    saveNextId(worldId + 1);

    std::string worldDir = getWorldDir(worldId);
    if (!fs::exists(worldDir)) {
        fs::create_directories(worldDir);
    }

    WorldMetadata meta{};
    meta.worldId = worldId;
    std::strncpy(meta.name, name.c_str(), sizeof(meta.name) - 1);
    std::strncpy(meta.baseMapPath, baseMapPath.c_str(), sizeof(meta.baseMapPath) - 1);

    auto now = std::chrono::system_clock::now();
    meta.createdAt =
            std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    meta.lastSavedAt = meta.createdAt;

    std::ofstream ofs(worldDir + "world_meta.bin", std::ios::binary | std::ios::trunc);
    if (ofs.is_open()) {
        ofs.write(reinterpret_cast<const char*>(&meta), sizeof(WorldMetadata));
    } else {
        std::cerr << "[WorldDataStore] Error creating world metadata file." << std::endl;
    }

    return worldId;
}

std::vector<WorldMetadata> WorldDataStore::listSavedWorlds() const {
    std::vector<WorldMetadata> worlds;
    for (const auto& entry: fs::directory_iterator(baseDir)) {
        if (entry.is_directory()) {
            std::string metaPath = entry.path().string() + "/world_meta.bin";
            if (fs::exists(metaPath)) {
                std::ifstream ifs(metaPath, std::ios::binary);
                if (ifs.is_open()) {
                    WorldMetadata meta;
                    if (ifs.read(reinterpret_cast<char*>(&meta), sizeof(WorldMetadata))) {
                        worlds.push_back(meta);
                    }
                }
            }
        }
    }
    return worlds;
}

std::optional<WorldMetadata> WorldDataStore::loadWorldMetadata(uint32_t worldId) const {
    std::string metaPath = getWorldDir(worldId) + "world_meta.bin";
    if (!fs::exists(metaPath)) {
        return std::nullopt;
    }

    std::ifstream ifs(metaPath, std::ios::binary);
    if (ifs.is_open()) {
        WorldMetadata meta;
        if (ifs.read(reinterpret_cast<char*>(&meta), sizeof(WorldMetadata))) {
            return meta;
        }
    }
    return std::nullopt;
}

void WorldDataStore::saveMonsters(uint32_t worldId,
                                  const std::vector<MonsterPersistData>& monsters) {
    std::string path = getWorldDir(worldId) + "monsters.bin";
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
        return;

    uint32_t count = monsters.size();
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
    if (count > 0) {
        ofs.write(reinterpret_cast<const char*>(monsters.data()),
                  count * sizeof(MonsterPersistData));
    }

    // Actualizar metadata lastSavedAt
    auto metaOpt = loadWorldMetadata(worldId);
    if (metaOpt) {
        WorldMetadata meta = metaOpt.value();
        auto now = std::chrono::system_clock::now();
        meta.lastSavedAt =
                std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::ofstream metaOfs(getWorldDir(worldId) + "world_meta.bin",
                              std::ios::binary | std::ios::trunc);
        if (metaOfs.is_open()) {
            metaOfs.write(reinterpret_cast<const char*>(&meta), sizeof(WorldMetadata));
        }
    }
}

std::vector<MonsterPersistData> WorldDataStore::loadMonsters(uint32_t worldId) const {
    std::vector<MonsterPersistData> monsters;
    std::string path = getWorldDir(worldId) + "monsters.bin";
    if (!fs::exists(path))
        return monsters;

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
        return monsters;

    uint32_t count = 0;
    if (ifs.read(reinterpret_cast<char*>(&count), sizeof(uint32_t)) && count > 0) {
        monsters.resize(count);
        ifs.read(reinterpret_cast<char*>(monsters.data()), count * sizeof(MonsterPersistData));
    }
    return monsters;
}

void WorldDataStore::saveNpcStates(uint32_t worldId,
                                   const std::vector<NpcHeaderPersistData>& headers,
                                   const std::vector<std::vector<NpcStockPersistData>>& allStocks) {
    std::string path = getWorldDir(worldId) + "npc_states.bin";
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
        return;

    uint32_t count = headers.size();
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));

    for (size_t i = 0; i < count; ++i) {
        ofs.write(reinterpret_cast<const char*>(&headers[i]), sizeof(NpcHeaderPersistData));

        if (headers[i].stockCount > 0) {
            ofs.write(reinterpret_cast<const char*>(allStocks[i].data()),
                      headers[i].stockCount * sizeof(NpcStockPersistData));
        }
    }

    auto metaOpt = loadWorldMetadata(worldId);
    if (metaOpt) {
        WorldMetadata meta = metaOpt.value();
        auto now = std::chrono::system_clock::now();
        meta.lastSavedAt =
                std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::ofstream metaOfs(getWorldDir(worldId) + "world_meta.bin",
                              std::ios::binary | std::ios::trunc);
        if (metaOfs.is_open()) {
            metaOfs.write(reinterpret_cast<const char*>(&meta), sizeof(WorldMetadata));
        }
    }
}

std::pair<std::vector<NpcHeaderPersistData>, std::vector<std::vector<NpcStockPersistData>>>
        WorldDataStore::loadNpcStates(uint32_t worldId) const {
    std::vector<NpcHeaderPersistData> headers;
    std::vector<std::vector<NpcStockPersistData>> allStocks;

    std::string path = getWorldDir(worldId) + "npc_states.bin";
    if (!fs::exists(path))
        return {headers, allStocks};

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
        return {headers, allStocks};

    uint32_t count = 0;
    if (ifs.read(reinterpret_cast<char*>(&count), sizeof(uint32_t)) && count > 0) {
        headers.resize(count);
        allStocks.resize(count);

        for (uint32_t i = 0; i < count; ++i) {
            ifs.read(reinterpret_cast<char*>(&headers[i]), sizeof(NpcHeaderPersistData));

            if (headers[i].stockCount > 0) {
                allStocks[i].resize(headers[i].stockCount);
                ifs.read(reinterpret_cast<char*>(allStocks[i].data()),
                         headers[i].stockCount * sizeof(NpcStockPersistData));
            }
        }
    }

    return {headers, allStocks};
}

void WorldDataStore::saveGroundItems(uint32_t worldId,
                                     const std::vector<GroundItemPersistData>& items) {
    std::string path = getWorldDir(worldId) + "ground_items.bin";
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
        return;

    uint32_t count = items.size();
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
    if (count > 0) {
        ofs.write(reinterpret_cast<const char*>(items.data()),
                  count * sizeof(GroundItemPersistData));
    }
}

std::vector<GroundItemPersistData> WorldDataStore::loadGroundItems(uint32_t worldId) const {
    std::vector<GroundItemPersistData> items;
    std::string path = getWorldDir(worldId) + "ground_items.bin";
    if (!fs::exists(path))
        return items;

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
        return items;

    uint32_t count = 0;
    if (ifs.read(reinterpret_cast<char*>(&count), sizeof(uint32_t)) && count > 0) {
        items.resize(count);
        ifs.read(reinterpret_cast<char*>(items.data()), count * sizeof(GroundItemPersistData));
    }
    return items;
}

void WorldDataStore::saveClans(uint32_t worldId, const std::vector<ClanHeaderPersistData>& headers,
                               const std::vector<std::vector<ClanPlayerPersistData>>& allMembers,
                               const std::vector<std::vector<ClanPlayerPersistData>>& allPending,
                               const std::vector<std::vector<ClanPlayerPersistData>>& allBanned) {
    std::string path = getWorldDir(worldId) + "clans.bin";
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
        return;

    uint32_t count = headers.size();
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));

    for (size_t i = 0; i < count; ++i) {
        ofs.write(reinterpret_cast<const char*>(&headers[i]), sizeof(ClanHeaderPersistData));
        if (headers[i].memberCount > 0) {
            ofs.write(reinterpret_cast<const char*>(allMembers[i].data()),
                      headers[i].memberCount * sizeof(ClanPlayerPersistData));
        }
        if (headers[i].pendingCount > 0) {
            ofs.write(reinterpret_cast<const char*>(allPending[i].data()),
                      headers[i].pendingCount * sizeof(ClanPlayerPersistData));
        }
        if (headers[i].bannedCount > 0) {
            ofs.write(reinterpret_cast<const char*>(allBanned[i].data()),
                      headers[i].bannedCount * sizeof(ClanPlayerPersistData));
        }
    }
}

std::tuple<std::vector<ClanHeaderPersistData>, std::vector<std::vector<ClanPlayerPersistData>>,
           std::vector<std::vector<ClanPlayerPersistData>>,
           std::vector<std::vector<ClanPlayerPersistData>>>
        WorldDataStore::loadClans(uint32_t worldId) const {

    std::vector<ClanHeaderPersistData> headers;
    std::vector<std::vector<ClanPlayerPersistData>> members;
    std::vector<std::vector<ClanPlayerPersistData>> pending;
    std::vector<std::vector<ClanPlayerPersistData>> banned;

    std::string path = getWorldDir(worldId) + "clans.bin";
    if (!fs::exists(path))
        return {headers, members, pending, banned};

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
        return {headers, members, pending, banned};

    uint32_t count = 0;
    if (ifs.read(reinterpret_cast<char*>(&count), sizeof(uint32_t)) && count > 0) {
        headers.resize(count);
        members.resize(count);
        pending.resize(count);
        banned.resize(count);

        for (uint32_t i = 0; i < count; ++i) {
            ifs.read(reinterpret_cast<char*>(&headers[i]), sizeof(ClanHeaderPersistData));

            if (headers[i].memberCount > 0) {
                members[i].resize(headers[i].memberCount);
                ifs.read(reinterpret_cast<char*>(members[i].data()),
                         headers[i].memberCount * sizeof(ClanPlayerPersistData));
            }
            if (headers[i].pendingCount > 0) {
                pending[i].resize(headers[i].pendingCount);
                ifs.read(reinterpret_cast<char*>(pending[i].data()),
                         headers[i].pendingCount * sizeof(ClanPlayerPersistData));
            }
            if (headers[i].bannedCount > 0) {
                banned[i].resize(headers[i].bannedCount);
                ifs.read(reinterpret_cast<char*>(banned[i].data()),
                         headers[i].bannedCount * sizeof(ClanPlayerPersistData));
            }
        }
    }
    return {headers, members, pending, banned};
}

void WorldDataStore::saveBankAccounts(uint32_t worldId,
                                      const std::vector<BankAccountHeaderPersistData>& headers,
                                      const std::vector<std::vector<BankSlotPersistData>>& slots) {
    std::string path = getWorldDir(worldId) + "bank_accounts.bin";
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
        return;

    uint32_t count = headers.size();
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));

    for (size_t i = 0; i < count; ++i) {
        ofs.write(reinterpret_cast<const char*>(&headers[i]), sizeof(BankAccountHeaderPersistData));
        if (headers[i].slotCount > 0) {
            ofs.write(reinterpret_cast<const char*>(slots[i].data()),
                      headers[i].slotCount * sizeof(BankSlotPersistData));
        }
    }
}

std::pair<std::vector<BankAccountHeaderPersistData>, std::vector<std::vector<BankSlotPersistData>>>
        WorldDataStore::loadBankAccounts(uint32_t worldId) const {

    std::vector<BankAccountHeaderPersistData> headers;
    std::vector<std::vector<BankSlotPersistData>> slots;

    std::string path = getWorldDir(worldId) + "bank_accounts.bin";
    if (!fs::exists(path))
        return {headers, slots};

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
        return {headers, slots};

    uint32_t count = 0;
    if (ifs.read(reinterpret_cast<char*>(&count), sizeof(uint32_t)) && count > 0) {
        headers.resize(count);
        slots.resize(count);

        for (uint32_t i = 0; i < count; ++i) {
            ifs.read(reinterpret_cast<char*>(&headers[i]), sizeof(BankAccountHeaderPersistData));

            if (headers[i].slotCount > 0) {
                slots[i].resize(headers[i].slotCount);
                ifs.read(reinterpret_cast<char*>(slots[i].data()),
                         headers[i].slotCount * sizeof(BankSlotPersistData));
            }
        }
    }
    return {headers, slots};
}
