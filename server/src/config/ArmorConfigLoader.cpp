#include "server/src/config/ArmorConfigLoader.h"

#include <toml++/toml.hpp>

#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace {

std::string requiredString(const toml::table& table, const std::string& fieldName) {
    const std::optional<std::string> value = table[fieldName].value<std::string>();
    if (!value.has_value() || value->empty()) {
        throw std::runtime_error("Missing or empty armor field: " + fieldName);
    }

    return *value;
}

int requiredInt(const toml::table& table, const std::string& fieldName) {
    const std::optional<int64_t> value = table[fieldName].value<int64_t>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing armor integer field: " + fieldName);
    }

    if (*value < std::numeric_limits<int>::min() || *value > std::numeric_limits<int>::max()) {
        throw std::runtime_error("Armor integer field out of range: " + fieldName);
    }

    return static_cast<int>(*value);
}

ArmorSlot parseSlot(const std::string& slot) {
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

ArmorConfig parseArmorConfig(const toml::table& armorTable) {
    ArmorConfig config{
            requiredInt(armorTable, "id"),
            parseSlot(requiredString(armorTable, "slot")),
            requiredInt(armorTable, "min_defense"),
            requiredInt(armorTable, "max_defense"),
    };

    if (config.minDefense > config.maxDefense) {
        throw std::runtime_error("Armor min_defense cannot exceed max_defense");
    }

    return config;
}

}  // namespace

std::unordered_map<std::string, ArmorConfig> ArmorConfigLoader::loadArmorConfigs(
        const std::filesystem::path& configPath) {
    toml::table config;
    try {
        config = toml::parse_file(configPath.string());
    } catch (const toml::parse_error& error) {
        throw std::runtime_error("Could not parse armor TOML config: " + std::string(error.what()));
    }

    const toml::array* armors = config["armor"].as_array();
    if (armors == nullptr) {
        throw std::runtime_error("Armor TOML config must define at least one [[armor]] table");
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

        if (!ids.insert(armorConfig.id).second) {
            throw std::runtime_error("Duplicated armor id in TOML config: " + std::to_string(armorConfig.id));
        }

        if (!armorConfigs.emplace(name, armorConfig).second) {
            throw std::runtime_error("Duplicated armor name in TOML config: " + name);
        }
    }

    return armorConfigs;
}

ArmorFactory ArmorConfigLoader::loadArmorFactory(const std::filesystem::path& configPath) {
    return ArmorFactory(loadArmorConfigs(configPath));
}
