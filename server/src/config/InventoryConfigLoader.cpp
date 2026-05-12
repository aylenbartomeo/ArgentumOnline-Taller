#include "server/src/config/InventoryConfigLoader.h"

#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include <toml++/toml.hpp>

namespace {

toml::table parseConfigFile(const std::filesystem::path& configPath) {
    try {
        return toml::parse_file(configPath.string());
    } catch (const toml::parse_error& error) {
        throw std::runtime_error("Could not parse inventory TOML config: " +
                                 std::string(error.what()));
    }
}

const toml::table& requiredTable(const toml::table& table, std::string_view fieldName) {
    const toml::table* child = table[fieldName].as_table();
    if (child == nullptr) {
        throw std::runtime_error("Missing inventory config table: " + std::string(fieldName));
    }
    return *child;
}

int requiredInt(const toml::table& table, std::string_view fieldName) {
    const std::optional<int64_t> value = table[fieldName].value<int64_t>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing inventory integer field: " + std::string(fieldName));
    }
    if (*value < std::numeric_limits<int>::min() || *value > std::numeric_limits<int>::max()) {
        throw std::runtime_error("Inventory integer field out of range: " + std::string(fieldName));
    }
    return static_cast<int>(*value);
}

uint8_t requiredUInt8(const toml::table& table, std::string_view fieldName) {
    const int value = requiredInt(table, fieldName);
    if (value < 0 || value > std::numeric_limits<uint8_t>::max()) {
        throw std::runtime_error("Inventory uint8 field out of range: " + std::string(fieldName));
    }
    return static_cast<uint8_t>(value);
}

}  // namespace

InventoryConfig InventoryConfigLoader::loadInventoryConfig(
        const std::filesystem::path& configPath) {
    const toml::table config = parseConfigFile(configPath);
    const toml::table& inventoryTable = requiredTable(config, "inventory");

    return InventoryConfig{requiredUInt8(inventoryTable, "max_slots")};
}
