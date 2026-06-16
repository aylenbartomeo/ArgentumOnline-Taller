#include "PlayerDataStore.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

PlayerDataStore::PlayerDataStore(const std::string& persistenceDir) {
    if (!fs::exists(persistenceDir)) {
        fs::create_directories(persistenceDir);
    }
    this->dataFilePath = persistenceDir + "players.bin";
    this->indexFilePath = persistenceDir + "players.idx";

    // Asegura que el archivo de datos exista si vamos a leer/escribir sin truncarlo
    if (!fs::exists(this->dataFilePath)) {
        std::ofstream ofs(this->dataFilePath, std::ios::binary | std::ios::out);
    }

    loadIndex();
}

void PlayerDataStore::loadIndex() {
    this->index.clear();

    if (!fs::exists(this->indexFilePath)) {
        return;
    }

    std::ifstream ifs(this->indexFilePath, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "[PlayerDataStore] Warning: Could not open index file for reading."
                  << std::endl;
        return;
    }

    PlayerIndexEntry entry;
    while (ifs.read(reinterpret_cast<char*>(&entry), sizeof(PlayerIndexEntry))) {
        // Asegura la terminación nula por si acaso
        entry.username[MAX_USERNAME_LENGTH - 1] = '\0';
        std::string uname(entry.username);
        this->index[uname] = entry.offset;
    }
}

void PlayerDataStore::flushIndex() {
    std::ofstream ofs(this->indexFilePath, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) {
        std::cerr << "[PlayerDataStore] Error: Could not open index file for writing." << std::endl;
        return;
    }

    for (const auto& [username, offset]: this->index) {
        PlayerIndexEntry entry{};
        std::strncpy(entry.username, username.c_str(), MAX_USERNAME_LENGTH - 1);
        entry.offset = offset;
        ofs.write(reinterpret_cast<const char*>(&entry), sizeof(PlayerIndexEntry));
    }
}

std::optional<PlayerPersistData> PlayerDataStore::loadPlayerData(
        const std::string& username) const {
    std::lock_guard<std::mutex> lock(this->storeMutex);
    auto it = this->index.find(username);
    if (it == this->index.end()) {
        return std::nullopt;
    }

    std::ifstream ifs(this->dataFilePath, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "[PlayerDataStore] Error: Could not open data file for reading." << std::endl;
        return std::nullopt;
    }

    ifs.seekg(it->second, std::ios::beg);

    PlayerPersistData data;
    if (ifs.read(reinterpret_cast<char*>(&data), sizeof(PlayerPersistData))) {
        return data;
    }

    return std::nullopt;
}

void PlayerDataStore::savePlayerData(const std::string& username, const PlayerPersistData& data) {
    std::lock_guard<std::mutex> lock(this->storeMutex);
    
    auto it = this->index.find(username);
    if (it != this->index.end()) {
        // El usuario existe, actualiza en el offset
        std::fstream fs(this->dataFilePath, std::ios::binary | std::ios::in | std::ios::out);
        if (!fs.is_open()) {
            std::cerr << "[PlayerDataStore] Error: Could not open data file for updating."
                      << std::endl;
            return;
        }

        fs.seekp(it->second, std::ios::beg);
        fs.write(reinterpret_cast<const char*>(&data), sizeof(PlayerPersistData));
    } else {
        // Nuevo usuario, añadir al final
        std::ofstream ofs(this->dataFilePath, std::ios::binary | std::ios::app);
        if (!ofs.is_open()) {
            std::cerr << "[PlayerDataStore] Error: Could not open data file for appending."
                      << std::endl;
            return;
        }

        // Obtiene el offset actual (que es el final del archivo)
        uint64_t currentOffset = ofs.tellp();
        ofs.write(reinterpret_cast<const char*>(&data), sizeof(PlayerPersistData));

        // Actualiza el índice y guarda
        this->index[username] = currentOffset;
        flushIndex();
    }
}
