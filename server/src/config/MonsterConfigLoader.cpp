#include "server/src/config/MonsterConfigLoader.h"

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
        throw std::runtime_error("Could not parse monster TOML config: " +
                                 std::string(error.what()));
    }
}

const toml::table& requiredTable(const toml::table& table, std::string_view fieldName) {
    const toml::table* child = table[fieldName].as_table();
    if (child == nullptr) {
        throw std::runtime_error("Missing monster config table: " + std::string(fieldName));
    }

    return *child;
}

int requiredInt(const toml::table& table, std::string_view fieldName) {
    const std::optional<int64_t> value = table[fieldName].value<int64_t>();
    if (!value.has_value()) {
        throw std::runtime_error("Missing monster integer field: " + std::string(fieldName));
    }

    if (*value < std::numeric_limits<int>::min() || *value > std::numeric_limits<int>::max()) {
        throw std::runtime_error("Monster integer field out of range: " + std::string(fieldName));
    }

    return static_cast<int>(*value);
}

std::string requiredString(const toml::table& table, std::string_view fieldName) {
    const std::optional<std::string> value = table[fieldName].value<std::string>();
    if (!value.has_value() || value->empty()) {
        throw std::runtime_error("Missing or empty monster string field: " +
                                 std::string(fieldName));
    }

    return *value;
}

MonsterConfig parseMonsterConfig(const toml::table& monsterTable) {
    MonsterConfig config{
            requiredInt(monsterTable, "max_health"),
            requiredInt(monsterTable, "strength"),
            requiredInt(monsterTable, "agility"),
            requiredInt(monsterTable, "attack_min"),
            requiredInt(monsterTable, "attack_max"),
            requiredInt(monsterTable, "detection_range"),
            requiredInt(monsterTable, "attack_range"),
            requiredInt(monsterTable, "min_level"),
            requiredInt(monsterTable, "max_level"),
            requiredString(monsterTable, "zone"),
            requiredInt(monsterTable, "attack_cooldown_ms"),
            requiredInt(monsterTable, "move_cooldown_ms"),
    };

    if (config.maxHealth <= 0) {
        throw std::runtime_error("Monster max_health must be positive");
    }

    if (config.attackMin > config.attackMax) {
        throw std::runtime_error("Monster attack_min cannot exceed attack_max");
    }

    if (config.detectionRange < 0 || config.attackRange < 1) {
        throw std::runtime_error("Monster ranges are invalid");
    }

    if (config.minLevel < 1 || config.minLevel > config.maxLevel) {
        throw std::runtime_error("Monster level ranges are invalid");
    }

    return config;
}

}  // namespace

MonsterConfigs MonsterConfigLoader::loadMonsterConfigs(const std::filesystem::path& configPath) {
    const toml::table config = parseConfigFile(configPath);
    const toml::table& monsters = requiredTable(config, "monsters");

    return MonsterConfigs{
            {NPCType::GOBLIN, parseMonsterConfig(requiredTable(monsters, "goblin"))},
            {NPCType::SKELETON, parseMonsterConfig(requiredTable(monsters, "skeleton"))},
            {NPCType::ZOMBIE, parseMonsterConfig(requiredTable(monsters, "zombie"))},
            {NPCType::SPIDER, parseMonsterConfig(requiredTable(monsters, "spider"))},
            {NPCType::ORC, parseMonsterConfig(requiredTable(monsters, "orc"))},
            {NPCType::GOLEM, parseMonsterConfig(requiredTable(monsters, "golem"))},
    };
}
