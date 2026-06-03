#include "WorldDataStore.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>
#include <chrono>

namespace fs = std::filesystem;

WorldDataStore::WorldDataStore(const std::string& worldsBaseDir) : baseDir(worldsBaseDir) {
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
    meta.createdAt = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
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
    for (const auto& entry : fs::directory_iterator(baseDir)) {
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

void WorldDataStore::saveMonsters(uint32_t worldId, const std::vector<MonsterPersistData>& monsters) {
    std::string path = getWorldDir(worldId) + "monsters.bin";
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) return;

    uint32_t count = monsters.size();
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
    if (count > 0) {
        ofs.write(reinterpret_cast<const char*>(monsters.data()), count * sizeof(MonsterPersistData));
    }
    
    // Actualizar metadata lastSavedAt
    auto metaOpt = loadWorldMetadata(worldId);
    if (metaOpt) {
        WorldMetadata meta = metaOpt.value();
        auto now = std::chrono::system_clock::now();
        meta.lastSavedAt = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::ofstream metaOfs(getWorldDir(worldId) + "world_meta.bin", std::ios::binary | std::ios::trunc);
        if (metaOfs.is_open()) {
            metaOfs.write(reinterpret_cast<const char*>(&meta), sizeof(WorldMetadata));
        }
    }
}

std::vector<MonsterPersistData> WorldDataStore::loadMonsters(uint32_t worldId) const {
    std::vector<MonsterPersistData> monsters;
    std::string path = getWorldDir(worldId) + "monsters.bin";
    if (!fs::exists(path)) return monsters;

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) return monsters;

    uint32_t count = 0;
    if (ifs.read(reinterpret_cast<char*>(&count), sizeof(uint32_t)) && count > 0) {
        monsters.resize(count);
        ifs.read(reinterpret_cast<char*>(monsters.data()), count * sizeof(MonsterPersistData));
    }
    return monsters;
}

void WorldDataStore::saveGroundItems(uint32_t worldId, const std::vector<GroundItemPersistData>& items) {
    std::string path = getWorldDir(worldId) + "ground_items.bin";
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) return;

    uint32_t count = items.size();
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
    if (count > 0) {
        ofs.write(reinterpret_cast<const char*>(items.data()), count * sizeof(GroundItemPersistData));
    }
}

std::vector<GroundItemPersistData> WorldDataStore::loadGroundItems(uint32_t worldId) const {
    std::vector<GroundItemPersistData> items;
    std::string path = getWorldDir(worldId) + "ground_items.bin";
    if (!fs::exists(path)) return items;

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) return items;

    uint32_t count = 0;
    if (ifs.read(reinterpret_cast<char*>(&count), sizeof(uint32_t)) && count > 0) {
        items.resize(count);
        ifs.read(reinterpret_cast<char*>(items.data()), count * sizeof(GroundItemPersistData));
    }
    return items;
}
