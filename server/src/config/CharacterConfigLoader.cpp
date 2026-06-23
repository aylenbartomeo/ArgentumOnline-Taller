#include "server/src/config/CharacterConfigLoader.h"

#include "server/src/config/TomlConfigHelper.h"

namespace {

constexpr std::string_view CTX = "character";

PlayerConfig parsePlayerConfig(const toml::table& playerTable) {
    return PlayerConfig{
            TomlHelper::requiredInt(playerTable, "base_strength", CTX),
            TomlHelper::requiredInt(playerTable, "base_intelligence", CTX),
            TomlHelper::requiredInt(playerTable, "base_agility", CTX),
            TomlHelper::requiredInt(playerTable, "base_constitution", CTX),
            TomlHelper::requiredUInt16(playerTable, "starting_level", CTX),
            TomlHelper::requiredUInt32(playerTable, "starting_experience", CTX),
            TomlHelper::requiredUInt32(playerTable, "starting_gold", CTX),
    };
}

RaceConfig parseRaceConfig(const toml::table& raceTable) {
    return RaceConfig{
            TomlHelper::requiredFloat(raceTable, "life_factor", CTX),
            TomlHelper::requiredFloat(raceTable, "mana_factor", CTX),
            TomlHelper::requiredFloat(raceTable, "strength_factor", CTX),
            TomlHelper::requiredFloat(raceTable, "agility_factor", CTX),
            TomlHelper::requiredFloat(raceTable, "intelligence_factor", CTX),
            TomlHelper::requiredFloat(raceTable, "recovery_factor", CTX),
    };
}

CharacterClassConfig parseClassConfig(const toml::table& classTable) {
    return CharacterClassConfig{
            TomlHelper::requiredFloat(classTable, "life_factor", CTX),
            TomlHelper::requiredFloat(classTable, "mana_factor", CTX),
            TomlHelper::requiredFloat(classTable, "meditation_factor", CTX),
            TomlHelper::requiredBool(classTable, "can_use_magic", CTX),
    };
}

}  // namespace

CharacterConfigs CharacterConfigLoader::loadCharacterConfigs(
        const std::filesystem::path& configPath) {
    const toml::table config = TomlHelper::parseConfigFile(configPath, CTX);
    const toml::table& races = TomlHelper::requiredTable(config, "races", CTX);
    const toml::table& classes = TomlHelper::requiredTable(config, "classes", CTX);

    return CharacterConfigs{
            parsePlayerConfig(TomlHelper::requiredTable(config, "player", CTX)),
            {
                    {Race::HUMAN, parseRaceConfig(TomlHelper::requiredTable(races, "human", CTX))},
                    {Race::ELF, parseRaceConfig(TomlHelper::requiredTable(races, "elf", CTX))},
                    {Race::DWARF, parseRaceConfig(TomlHelper::requiredTable(races, "dwarf", CTX))},
                    {Race::GNOME, parseRaceConfig(TomlHelper::requiredTable(races, "gnome", CTX))},
            },
            {
                    {CharacterClass::MAGE,
                     parseClassConfig(TomlHelper::requiredTable(classes, "mage", CTX))},
                    {CharacterClass::WARRIOR,
                     parseClassConfig(TomlHelper::requiredTable(classes, "warrior", CTX))},
                    {CharacterClass::PALADIN,
                     parseClassConfig(TomlHelper::requiredTable(classes, "paladin", CTX))},
                    {CharacterClass::CLERIC,
                     parseClassConfig(TomlHelper::requiredTable(classes, "cleric", CTX))},
            },
    };
}
