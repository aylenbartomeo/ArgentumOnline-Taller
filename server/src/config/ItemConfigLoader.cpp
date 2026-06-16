#include "server/src/config/ItemConfigLoader.h"

#include <unordered_map>
#include <unordered_set>

#include "server/src/config/TomlConfigHelper.h"

namespace {

constexpr std::string_view CTX = "item";

ArmorSlot parseArmorSlot(const std::string& slot) {
    if (slot == "body")
        return ArmorSlot::Body;
    if (slot == "helmet")
        return ArmorSlot::Head;
    if (slot == "shield")
        return ArmorSlot::Shield;
    throw std::runtime_error("Unknown armor slot: " + slot);
}

WeaponType parseWeaponType(const std::string& type) {
    if (type == "melee")
        return WeaponType::MELEE;
    if (type == "ranged")
        return WeaponType::RANGED;
    if (type == "magic")
        return WeaponType::MAGIC;
    throw std::runtime_error("Unknown weapon type: " + type);
}

ConsumableType parseConsumableType(const std::string& type) {
    if (type == "health")
        return ConsumableType::HEALTH;
    if (type == "mana")
        return ConsumableType::MANA;
    if (type == "boostStr")
        return ConsumableType::BOOST_STR;
    if (type == "boostAgi")
        return ConsumableType::BOOST_AGI;
    throw std::runtime_error("Unknown consumable type: " + type);
}

ArmorConfig parseArmorConfig(const toml::table& armorTable) {
    ArmorConfig config{
            TomlHelper::requiredInt(armorTable, "id", CTX),
            TomlHelper::requiredInt(armorTable, "price", CTX),
            parseArmorSlot(TomlHelper::requiredString(armorTable, "slot", CTX)),
            TomlHelper::requiredInt(armorTable, "min_defense", CTX),
            TomlHelper::requiredInt(armorTable, "max_defense", CTX),
    };

    if (config.minDefense > config.maxDefense) {
        throw std::runtime_error("Armor min_defense cannot exceed max_defense");
    }
    return config;
}

WeaponConfig parseWeaponConfig(const toml::table& weaponTable) {
    WeaponConfig config{
            TomlHelper::requiredInt(weaponTable, "id", CTX),
            TomlHelper::requiredInt(weaponTable, "price", CTX),
            parseWeaponType(TomlHelper::requiredString(weaponTable, "type", CTX)),
            TomlHelper::requiredInt(weaponTable, "min_damage", CTX),
            TomlHelper::requiredInt(weaponTable, "max_damage", CTX),
            TomlHelper::requiredInt(weaponTable, "attack_range", CTX),
            TomlHelper::requiredInt(weaponTable, "mana_cost", CTX),
            weaponTable["delivery"].value<std::string>().value_or(""),
            weaponTable["hit_effect"].value<std::string>().value_or(""),
    };

    if (config.minDamage > config.maxDamage) {
        throw std::runtime_error("Weapon min_damage cannot exceed max_damage");
    }
    return config;
}

ConsumableConfig parseConsumableConfig(const toml::table& consumableTable) {
    return ConsumableConfig{
            TomlHelper::requiredInt(consumableTable, "id", CTX),
            TomlHelper::requiredInt(consumableTable, "price", CTX),
            parseConsumableType(TomlHelper::requiredString(consumableTable, "type", CTX)),
            TomlHelper::requiredInt(consumableTable, "durationMs", CTX),
            TomlHelper::requiredInt(consumableTable, "effectValue", CTX),
    };
}

void validateUniqueId(std::unordered_set<int>& ids, const int id) {
    if (!ids.insert(id).second) {
        throw std::runtime_error("Duplicated item id in TOML config: " + std::to_string(id));
    }
}

// Helper interno para evitar duplicar la lógica de bucle en armors, weapons y consumables
template <typename T, typename ParseFunc>
std::unordered_map<std::string, T> loadGenericConfigs(const toml::table& config,
                                                      std::string_view arrayName,
                                                      ParseFunc parseFunc) {
    const toml::array& itemsArray = TomlHelper::requiredArray(config, arrayName, CTX);
    std::unordered_map<std::string, T> configsMap;
    std::unordered_set<int> ids;

    for (const toml::node& node: itemsArray) {
        const toml::table* itemTable = node.as_table();
        if (itemTable == nullptr) {
            throw std::runtime_error("Each entry in [[" + std::string(arrayName) +
                                     "]] must be a TOML table");
        }

        const std::string name = TomlHelper::requiredString(*itemTable, "name", CTX);
        T itemConfig = parseFunc(*itemTable);
        validateUniqueId(ids, itemConfig.id);

        if (!configsMap.emplace(name, itemConfig).second) {
            throw std::runtime_error("Duplicated item name in TOML config: " + name);
        }
    }
    return configsMap;
}

}  // namespace

std::unordered_map<std::string, ArmorConfig> ItemConfigLoader::loadArmorConfigs(
        const std::filesystem::path& configPath) {
    const toml::table config = TomlHelper::parseConfigFile(configPath, CTX);
    return loadGenericConfigs<ArmorConfig>(config, "armor", parseArmorConfig);
}

std::unordered_map<std::string, WeaponConfig> ItemConfigLoader::loadWeaponConfigs(
        const std::filesystem::path& configPath) {
    const toml::table config = TomlHelper::parseConfigFile(configPath, CTX);
    return loadGenericConfigs<WeaponConfig>(config, "weapon", parseWeaponConfig);
}

std::unordered_map<std::string, ConsumableConfig> ItemConfigLoader::loadConsumableConfigs(
        const std::filesystem::path& configPath) {
    const toml::table config = TomlHelper::parseConfigFile(configPath, CTX);
    return loadGenericConfigs<ConsumableConfig>(config, "consumable", parseConsumableConfig);
}

ArmorFactory ItemConfigLoader::loadArmorFactory(const std::filesystem::path& configPath) {
    return ArmorFactory(loadArmorConfigs(configPath));
}

WeaponFactory ItemConfigLoader::loadWeaponFactory(const std::filesystem::path& configPath) {
    return WeaponFactory(loadWeaponConfigs(configPath));
}

ConsumableFactory ItemConfigLoader::loadConsumableFactory(const std::filesystem::path& configPath) {
    return ConsumableFactory(loadConsumableConfigs(configPath));
}

ItemFactories ItemConfigLoader::loadItemFactories(const std::filesystem::path& configPath) {
    return ItemFactories{
            loadArmorFactory(configPath),
            loadWeaponFactory(configPath),
            loadConsumableFactory(configPath),
    };
}
