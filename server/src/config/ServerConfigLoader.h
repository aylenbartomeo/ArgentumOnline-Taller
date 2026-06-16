#pragma once

#include <filesystem>
#include <string>

#include "server/src/config/TomlConfigHelper.h"

struct ServerConfig {
    // Servidor / Red
    int port;
    std::string worldName;
    std::string mapPath;

    // Balance de Combate
    float criticalProbability;

    // Balance de Clanes
    int clanBonusRange;
    float clanAttackBonusPerMember;
    float clanDefenseBonusPerMember;
};

ServerConfig loadServerConfig(const std::filesystem::path& path);
