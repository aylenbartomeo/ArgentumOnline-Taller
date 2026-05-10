#include "server/src/config/ItemConfigLoader.h"

#include <toml++/toml.hpp>

#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace {

toml::table parseConfigFile(const std::filesystem::path& configPath) {
    try {
        return toml::parse_file(configPath.string());
    } catch (const toml::parse_error& error) {
        throw std::runtime_error("Could not parse item TOML config: " + std::string(error.what()));
    }
}

std::string requiredString(const toml::table& table, const std::string& fieldName) {
    const std::optional<std::string> value = table[fieldName].value<std::string>();
    if (!value.has_value() || value->empty()) {
        throw std::runtime_error("Missing or empty item field: " + fieldName);
    }

    return *value;
}

int requiredInt(const toml::table& table, const std::string& fieldName) {
    const std::optional<int64_t> value = table[fieldName].value<int64_t>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing item integer field: " + fieldName);
    }

    if (*value < std::numeric_limits<int>::min() || *value > std::numeric_limits<int>::max()) {
        throw std::runtime_error("Item integer field out of range: " + fieldName);
    }

    return static_cast<int>(*value);
}

ArmorSlot parseArmorSlot(const std::string& slot) {
    if (slot == "body") {
        return ArmorSlot::Body;
    }

    if (slot == "helmet") {
        return ArmorSlot::Head;
    }

    if (slot == "shield") {
        return ArmorSlot::Shield;
    }

    throw std::runtime_error("Unknown armor slot: " + slot);
}

WeaponType parseWeaponType(const std::string& type) {
    if (type == "melee") {
        return WeaponType::MELEE;
    }

    if (type == "ranged") {
        return WeaponType::RANGED;
    }

    if (type == "magic") {
        return WeaponType::MAGIC;
    }

    throw std::runtime_error("Unknown weapon type: " + type);
}

ArmorConfig parseArmorConfig(const toml::table& armorTable) {
    ArmorConfig config{
            requiredInt(armorTable, "id"),
            parseArmorSlot(requiredString(armorTable, "slot")),
            requiredInt(armorTable, "min_defense"),
            requiredInt(armorTable, "max_defense"),
    };

    if (config.minDefense > config.maxDefense) {
        throw std::runtime_error("Armor min_defense cannot exceed max_defense");
    }

    return config;
}

WeaponConfig parseWeaponConfig(const toml::table& weaponTable) {
    WeaponConfig config{
            requiredInt(weaponTable, "id"),
            parseWeaponType(requiredString(weaponTable, "type")),
            requiredInt(weaponTable, "min_damage"),
            requiredInt(weaponTable, "max_damage"),
            requiredInt(weaponTable, "attack_range"),
            requiredInt(weaponTable, "mana_cost"),
    };

    if (config.minDamage > config.maxDamage) {
        throw std::runtime_error("Weapon min_damage cannot exceed max_damage");
    }

    return config;
}

void validateUniqueId(std::unordered_set<int>& ids, const int id) {
    if (!ids.insert(id).second) {
        throw std::runtime_error("Duplicated item id in TOML config: " + std::to_string(id));
    }
}

}  // namespace

std::unordered_map<std::string, ArmorConfig> ItemConfigLoader::loadArmorConfigs(
        const std::filesystem::path& configPath) {
    const toml::table config = parseConfigFile(configPath);

    const toml::array* armors = config["armor"].as_array();
    if (armors == nullptr) {
        throw std::runtime_error("Item TOML config must define at least one [[armor]] table");
    }

    std::unordered_map<std::string, ArmorConfig> armorConfigs;
    std::unordered_set<int> ids;

    for (const toml::node& armorNode: *armors) {
        const toml::table* armorTable = armorNode.as_table();
        if (armorTable == nullptr) {
            throw std::runtime_error("Each armor entry must be a TOML table");
        }

        const std::string name = requiredString(*armorTable, "name");
        ArmorConfig armorConfig = parseArmorConfig(*armorTable);
        validateUniqueId(ids, armorConfig.id);

        if (!armorConfigs.emplace(name, armorConfig).second) {
            throw std::runtime_error("Duplicated armor name in TOML config: " + name);
        }
    }

    return armorConfigs;
}

std::unordered_map<std::string, WeaponConfig> ItemConfigLoader::loadWeaponConfigs(
        const std::filesystem::path& configPath) {
    const toml::table config = parseConfigFile(configPath);

    const toml::array* weapons = config["weapon"].as_array();
    if (weapons == nullptr) {
        throw std::runtime_error("Item TOML config must define at least one [[weapon]] table");
    }

    std::unordered_map<std::string, WeaponConfig> weaponConfigs;
    std::unordered_set<int> ids;

    for (const toml::node& weaponNode: *weapons) {
        const toml::table* weaponTable = weaponNode.as_table();
        if (weaponTable == nullptr) {
            throw std::runtime_error("Each weapon entry must be a TOML table");
        }

        const std::string name = requiredString(*weaponTable, "name");
        WeaponConfig weaponConfig = parseWeaponConfig(*weaponTable);
        validateUniqueId(ids, weaponConfig.id);

        if (!weaponConfigs.emplace(name, weaponConfig).second) {
            throw std::runtime_error("Duplicated weapon name in TOML config: " + name);
        }
    }

    return weaponConfigs;
}

ArmorFactory ItemConfigLoader::loadArmorFactory(const std::filesystem::path& configPath) {
    return ArmorFactory(loadArmorConfigs(configPath));
}

WeaponFactory ItemConfigLoader::loadWeaponFactory(const std::filesystem::path& configPath) {
    return WeaponFactory(loadWeaponConfigs(configPath));
}

ItemFactories ItemConfigLoader::loadItemFactories(const std::filesystem::path& configPath) {
    return ItemFactories{
            loadArmorFactory(configPath),
            loadWeaponFactory(configPath),
    };
}
