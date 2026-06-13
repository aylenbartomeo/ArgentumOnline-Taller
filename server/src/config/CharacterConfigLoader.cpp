#include "server/src/config/CharacterConfigLoader.h"

#include <cstdint>
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
        throw std::runtime_error("Could not parse character TOML config: " +
                                 std::string(error.what()));
    }
}

const toml::table& requiredTable(const toml::table& table, std::string_view fieldName) {
    const toml::table* child = table[fieldName].as_table();
    if (child == nullptr) {
        throw std::runtime_error("Missing character config table: " + std::string(fieldName));
    }

    return *child;
}

int requiredInt(const toml::table& table, std::string_view fieldName) {
    const std::optional<int64_t> value = table[fieldName].value<int64_t>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing character integer field: " + std::string(fieldName));
    }

    if (*value < std::numeric_limits<int>::min() || *value > std::numeric_limits<int>::max()) {
        throw std::runtime_error("Character integer field out of range: " + std::string(fieldName));
    }

    return static_cast<int>(*value);
}

uint16_t requiredUInt16(const toml::table& table, std::string_view fieldName) {
    const int value = requiredInt(table, fieldName);
    if (value < 0 || value > std::numeric_limits<uint16_t>::max()) {
        throw std::runtime_error("Character uint16 field out of range: " + std::string(fieldName));
    }

    return static_cast<uint16_t>(value);
}

uint32_t requiredUInt32(const toml::table& table, std::string_view fieldName) {
    const std::optional<int64_t> value = table[fieldName].value<int64_t>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing character uint32 field: " + std::string(fieldName));
    }

    if (*value < 0 || *value > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("Character uint32 field out of range: " + std::string(fieldName));
    }

    return static_cast<uint32_t>(*value);
}

float requiredFloat(const toml::table& table, std::string_view fieldName) {
    const std::optional<double> value = table[fieldName].value<double>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing character float field: " + std::string(fieldName));
    }

    return static_cast<float>(*value);
}

bool requiredBool(const toml::table& table, std::string_view fieldName) {
    const std::optional<bool> value = table[fieldName].value<bool>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing character bool field: " + std::string(fieldName));
    }

    return *value;
}

PlayerConfig parsePlayerConfig(const toml::table& playerTable) {
    return PlayerConfig{
            requiredInt(playerTable, "base_strength"),
            requiredInt(playerTable, "base_intelligence"),
            requiredInt(playerTable, "base_agility"),
            requiredInt(playerTable, "base_constitution"),
            requiredUInt16(playerTable, "starting_level"),
            requiredUInt32(playerTable, "starting_experience"),
            requiredUInt32(playerTable, "starting_gold"),
    };
}

RaceConfig parseRaceConfig(const toml::table& raceTable) {
    return RaceConfig{
            requiredFloat(raceTable, "life_factor"),
            requiredFloat(raceTable, "mana_factor"),
            requiredFloat(raceTable, "strength_factor"),
            requiredFloat(raceTable, "agility_factor"),
            requiredFloat(raceTable, "intelligence_factor"),
            requiredFloat(raceTable, "recovery_factor"),
    };
}

CharacterClassConfig parseClassConfig(const toml::table& classTable) {
    return CharacterClassConfig{
            requiredFloat(classTable, "life_factor"),
            requiredFloat(classTable, "mana_factor"),
            requiredFloat(classTable, "meditation_factor"),
            requiredBool(classTable, "can_use_magic"),
    };
}

}  // namespace

CharacterConfigs CharacterConfigLoader::loadCharacterConfigs(
        const std::filesystem::path& configPath) {
    const toml::table config = parseConfigFile(configPath);
    const toml::table& races = requiredTable(config, "races");
    const toml::table& classes = requiredTable(config, "classes");

    return CharacterConfigs{
            parsePlayerConfig(requiredTable(config, "player")),
            {
                    {Race::HUMAN, parseRaceConfig(requiredTable(races, "human"))},
                    {Race::ELF, parseRaceConfig(requiredTable(races, "elf"))},
                    {Race::DWARF, parseRaceConfig(requiredTable(races, "dwarf"))},
                    {Race::GNOME, parseRaceConfig(requiredTable(races, "gnome"))},
            },
            {
                    {CharacterClass::MAGE, parseClassConfig(requiredTable(classes, "mage"))},
                    {CharacterClass::WARRIOR, parseClassConfig(requiredTable(classes, "warrior"))},
                    {CharacterClass::PALADIN, parseClassConfig(requiredTable(classes, "paladin"))},
                    {CharacterClass::CLERIC, parseClassConfig(requiredTable(classes, "cleric"))},
            },
    };
}
