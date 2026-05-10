#include "server/src/config/ArmorConfigLoader.h"

#include "server/src/config/ItemConfigLoader.h"

std::unordered_map<std::string, ArmorConfig> ArmorConfigLoader::loadArmorConfigs(
        const std::filesystem::path& configPath) {
    return ItemConfigLoader::loadArmorConfigs(configPath);
}

ArmorFactory ArmorConfigLoader::loadArmorFactory(const std::filesystem::path& configPath) {
    return ArmorFactory(loadArmorConfigs(configPath));
}
