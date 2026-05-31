#ifndef SERVER_SRC_CONFIG_ITEMCONFIGLOADER_H
#define SERVER_SRC_CONFIG_ITEMCONFIGLOADER_H

#include <filesystem>
#include <string>
#include <unordered_map>

#include "server/src/model/items/ArmorFactory.h"
#include "server/src/model/items/ConsumableFactory.h"
#include "server/src/model/items/WeaponFactory.h"

struct ItemFactories {
    ArmorFactory armorFactory;
    WeaponFactory weaponFactory;
    ConsumableFactory consumableFactory;
};

class ItemConfigLoader {
public:
    static std::unordered_map<std::string, ArmorConfig> loadArmorConfigs(
            const std::filesystem::path& configPath);

    static std::unordered_map<std::string, WeaponConfig> loadWeaponConfigs(
            const std::filesystem::path& configPath);

    static std::unordered_map<std::string, ConsumableConfig> loadConsumableConfigs(
        const std::filesystem::path& configPath);

    static ArmorFactory loadArmorFactory(const std::filesystem::path& configPath);
    static WeaponFactory loadWeaponFactory(const std::filesystem::path& configPath);
    static ConsumableFactory loadConsumableFactory(const std::filesystem::path& configPath);
    static ItemFactories loadItemFactories(const std::filesystem::path& configPath);
};

#endif
