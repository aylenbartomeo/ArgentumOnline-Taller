#ifndef TOML_CONFIG_HELPER_H
#define TOML_CONFIG_HELPER_H

#include <filesystem>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <toml++/toml.hpp>

namespace TomlHelper {

inline toml::table parseConfigFile(const std::filesystem::path& configPath,
                                   std::string_view context) {
    try {
        return toml::parse_file(configPath.string());
    } catch (const toml::parse_error& error) {
        throw std::runtime_error("Could not parse " + std::string(context) +
                                 " TOML config: " + std::string(error.what()));
    }
}

inline const toml::table& requiredTable(const toml::table& table, std::string_view fieldName,
                                        std::string_view context) {
    const toml::table* child = table[fieldName].as_table();
    if (child == nullptr) {
        throw std::runtime_error("Missing " + std::string(context) +
                                 " config table: " + std::string(fieldName));
    }
    return *child;
}

inline int requiredInt(const toml::table& table, std::string_view fieldName,
                       std::string_view context) {
    const std::optional<int64_t> value = table[fieldName].value<int64_t>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing " + std::string(context) +
                                 " integer field: " + std::string(fieldName));
    }
    if (*value < std::numeric_limits<int>::min() || *value > std::numeric_limits<int>::max()) {
        throw std::runtime_error(std::string(context) +
                                 " integer field out of range: " + std::string(fieldName));
    }
    return static_cast<int>(*value);
}

inline uint8_t requiredUInt8(const toml::table& table, std::string_view fieldName,
                             std::string_view context) {
    const int value = requiredInt(table, fieldName, context);
    if (value < 0 || value > std::numeric_limits<uint8_t>::max()) {
        throw std::runtime_error(std::string(context) +
                                 " uint8 field out of range: " + std::string(fieldName));
    }
    return static_cast<uint8_t>(value);
}

inline uint16_t requiredUInt16(const toml::table& table, std::string_view fieldName,
                               std::string_view context) {
    const int value = requiredInt(table, fieldName, context);
    if (value < 0 || value > std::numeric_limits<uint16_t>::max()) {
        throw std::runtime_error(std::string(context) +
                                 " uint16 field out of range: " + std::string(fieldName));
    }
    return static_cast<uint16_t>(value);
}

inline uint32_t requiredUInt32(const toml::table& table, std::string_view fieldName,
                               std::string_view context) {
    const std::optional<int64_t> value = table[fieldName].value<int64_t>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing " + std::string(context) +
                                 " uint32 field: " + std::string(fieldName));
    }
    if (*value < 0 || *value > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error(std::string(context) +
                                 " uint32 field out of range: " + std::string(fieldName));
    }
    return static_cast<uint32_t>(*value);
}

inline float requiredFloat(const toml::table& table, std::string_view fieldName,
                           std::string_view context) {
    const std::optional<double> value = table[fieldName].value<double>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing " + std::string(context) +
                                 " float field: " + std::string(fieldName));
    }
    return static_cast<float>(*value);
}

inline bool requiredBool(const toml::table& table, std::string_view fieldName,
                         std::string_view context) {
    const std::optional<bool> value = table[fieldName].value<bool>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing " + std::string(context) +
                                 " bool field: " + std::string(fieldName));
    }
    return *value;
}

inline std::string requiredString(const toml::table& table, std::string_view fieldName,
                                  std::string_view context) {
    const std::optional<std::string> value = table[fieldName].value<std::string>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing " + std::string(context) +
                                 " string field: " + std::string(fieldName));
    }
    return *value;
}

inline const toml::array& requiredArray(const toml::table& table, std::string_view fieldName,
                                        std::string_view context) {
    const toml::array* arr = table[fieldName].as_array();
    if (arr == nullptr) {
        throw std::runtime_error(std::string(context) + " TOML config must define a valid [[" +
                                 std::string(fieldName) + "]] array");
    }
    return *arr;
}

inline bool optionalBool(const toml::table& table, std::string_view fieldName,
                         bool defaultValue = false) {
    return table[fieldName].value_or(defaultValue);
}

inline uint32_t optionalUInt32(const toml::table& table, std::string_view fieldName,
                               uint32_t defaultValue = 0) {
    const std::optional<int64_t> value = table[fieldName].value<int64_t>();
    if (!value.has_value()) {
        return defaultValue;
    }
    return static_cast<uint32_t>(*value);
}

inline std::vector<uint32_t> optionalUInt32Array(const toml::table& table,
                                                 std::string_view fieldName) {
    std::vector<uint32_t> result;
    if (const toml::array* arr = table[fieldName].as_array()) {
        for (const auto& elem: *arr) {
            if (auto val = elem.value<int64_t>()) {
                result.push_back(static_cast<uint32_t>(*val));
            }
        }
    }
    return result;
}

}  // namespace TomlHelper

#endif
