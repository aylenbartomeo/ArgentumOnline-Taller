#include "server/src/config/MonsterConfigLoader.h"

#include "server/src/config/TomlConfigHelper.h"

namespace {

constexpr std::string_view CTX = "monster";

MonsterConfig parseMonsterConfig(const toml::table& monsterTable) {
    MonsterConfig config{
            TomlHelper::requiredInt(monsterTable, "max_health", CTX),
            TomlHelper::requiredInt(monsterTable, "strength", CTX),
            TomlHelper::requiredInt(monsterTable, "agility", CTX),
            TomlHelper::requiredInt(monsterTable, "attack_min", CTX),
            TomlHelper::requiredInt(monsterTable, "attack_max", CTX),
            TomlHelper::requiredInt(monsterTable, "detection_range", CTX),
            TomlHelper::requiredInt(monsterTable, "attack_range", CTX),
            TomlHelper::requiredInt(monsterTable, "min_level", CTX),
            TomlHelper::requiredInt(monsterTable, "max_level", CTX),
            TomlHelper::requiredString(monsterTable, "zone", CTX),
            TomlHelper::requiredInt(monsterTable, "attack_cooldown_ms", CTX),
            TomlHelper::requiredInt(monsterTable, "move_cooldown_ms", CTX),
            TomlHelper::optionalBool(monsterTable, "is_boss"),
            TomlHelper::optionalUInt32(monsterTable, "unique_drop_item_id"),
            TomlHelper::optionalUInt32(monsterTable, "guaranteed_gold"),
            TomlHelper::optionalUInt32Array(monsterTable, "extra_loot"),
    };

    // ─── Validaciones lógicas específicas del negocio ────────────────────────
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
    const toml::table config = TomlHelper::parseConfigFile(configPath, CTX);
    const toml::table& monsters = TomlHelper::requiredTable(config, "monsters", CTX);

    return MonsterConfigs{
            {NPCType::GOBLIN,
             parseMonsterConfig(TomlHelper::requiredTable(monsters, "goblin", CTX))},
            {NPCType::SKELETON,
             parseMonsterConfig(TomlHelper::requiredTable(monsters, "skeleton", CTX))},
            {NPCType::ZOMBIE,
             parseMonsterConfig(TomlHelper::requiredTable(monsters, "zombie", CTX))},
            {NPCType::SPIDER,
             parseMonsterConfig(TomlHelper::requiredTable(monsters, "spider", CTX))},
            {NPCType::ORC, parseMonsterConfig(TomlHelper::requiredTable(monsters, "orc", CTX))},
            {NPCType::GOLEM, parseMonsterConfig(TomlHelper::requiredTable(monsters, "golem", CTX))},
            {NPCType::BOSS_BALROG,
             parseMonsterConfig(TomlHelper::requiredTable(monsters, "boss_balrog", CTX))},
            {NPCType::BOSS_TITAN,
             parseMonsterConfig(TomlHelper::requiredTable(monsters, "boss_titan", CTX))},
            {NPCType::BOSS_COLOSO,
             parseMonsterConfig(TomlHelper::requiredTable(monsters, "boss_coloso", CTX))},
            {NPCType::BOSS_ARACNE,
             parseMonsterConfig(TomlHelper::requiredTable(monsters, "boss_aracne", CTX))},
    };
}
