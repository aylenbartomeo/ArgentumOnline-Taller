#ifndef SERVER_SRC_CONFIG_ARMORCONFIGLOADER_H
#define SERVER_SRC_CONFIG_ARMORCONFIGLOADER_H

#include "server/src/model/items/ArmorFactory.h"

#include <filesystem>
#include <string>
#include <unordered_map>

class ArmorConfigLoader {
public:
    static std::unordered_map<std::string, ArmorConfig> loadArmorConfigs(
            const std::filesystem::path& configPath);

    static ArmorFactory loadArmorFactory(const std::filesystem::path& configPath);
};

#endif
