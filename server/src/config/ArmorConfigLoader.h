#ifndef SERVER_SRC_CONFIG_ARMORCONFIGLOADER_H
#define SERVER_SRC_CONFIG_ARMORCONFIGLOADER_H

#include <filesystem>
#include <string>
#include <unordered_map>

#include "server/src/model/items/ArmorFactory.h"

class ArmorConfigLoader {
public:
    static std::unordered_map<std::string, ArmorConfig> loadArmorConfigs(
            const std::filesystem::path& configPath);

    static ArmorFactory loadArmorFactory(const std::filesystem::path& configPath);
};

#endif
