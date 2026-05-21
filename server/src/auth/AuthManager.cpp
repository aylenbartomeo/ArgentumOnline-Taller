#include "AuthManager.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

#include <bcrypt/BCrypt.hpp>
#include <toml++/toml.hpp>

namespace fs = std::filesystem;

AuthManager::AuthManager(const std::string& usersDir): usersDirectory(usersDir), nextId(1) {
    if (!fs::exists(usersDirectory)) {
        fs::create_directories(usersDirectory);
    }
    initializeNextId();
}

void AuthManager::initializeNextId() {
    uint32_t max_id = 0;
    for (const auto& entry: fs::directory_iterator(usersDirectory)) {
        if (entry.path().extension() == ".toml") {
            try {
                toml::table config = toml::parse_file(entry.path().string());
                auto idNode = config["user"]["id"].value<uint32_t>();
                if (idNode && *idNode > max_id) {
                    max_id = *idNode;
                }
            } catch (...) {}
        }
    }
    nextId = max_id + 1;
    // std::cout << "[AUTH] Database initialized. Next available ID: " << nextId << std::endl;
}

std::string AuthManager::getUserFilePath(const std::string& username) const {
    return usersDirectory + username + ".toml";
}

bool AuthManager::userExists(const std::string& username) const {
    return fs::exists(getUserFilePath(username));
}

std::optional<uint32_t> AuthManager::registerUser(const std::string& username,
                                                  const std::string& password) {
    if (userExists(username))
        return std::nullopt;

    std::string hashedPassword = BCrypt::generateHash(password);
    uint32_t newId = nextId.fetch_add(1);  // Suma 1 en RAM de forma atómica

    toml::table userTable;
    userTable.insert("id", newId);
    userTable.insert("username", username);
    userTable.insert("password", hashedPassword);

    toml::table rootTable;
    rootTable.insert("user", userTable);

    std::ofstream file(getUserFilePath(username));
    if (file.is_open()) {
        file << rootTable;
        return newId;
    }
    return std::nullopt;
}

std::optional<uint32_t> AuthManager::validateUser(const std::string& username,
                                                  const std::string& password) const {
    if (!userExists(username))
        return std::nullopt;

    try {
        toml::table config = toml::parse_file(getUserFilePath(username));

        auto savedHash = config["user"]["password"].value<std::string>();
        auto savedId = config["user"]["id"].value<uint32_t>();

        if (!savedHash || !savedId)
            return std::nullopt;

        if (BCrypt::validatePassword(password, *savedHash)) {
            return *savedId;
        }
    } catch (...) {}

    return std::nullopt;
}
