#ifndef SERVER_SRC_CONFIG_ITEMCONFIGLOADER_H
#define SERVER_SRC_CONFIG_ITEMCONFIGLOADER_H

#include "server/src/model/items/ArmorFactory.h"
#include "server/src/model/items/WeaponFactory.h"

#include <filesystem>
#include <string>
#include <unordered_map>

struct ItemFactories {
    ArmorFactory armorFactory;
    WeaponFactory weaponFactory;
};

class ItemConfigLoader {
public:
    static std::unordered_map<std::string, ArmorConfig> loadArmorConfigs(
            const std::filesystem::path& configPath);

    static std::unordered_map<std::string, WeaponConfig> loadWeaponConfigs(
            const std::filesystem::path& configPath);

    static ArmorFactory loadArmorFactory(const std::filesystem::path& configPath);
    static WeaponFactory loadWeaponFactory(const std::filesystem::path& configPath);
    static ItemFactories loadItemFactories(const std::filesystem::path& configPath);
};

#endif
