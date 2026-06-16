#include "ServerConfigLoader.h"
#include "server/src/config/TomlConfigHelper.h"
#include <string_view>

ServerConfig loadServerConfig(const std::filesystem::path& path) {
    constexpr std::string_view CTX = "server_system";
    
    toml::table config = TomlHelper::parseConfigFile(path, CTX);
    
    const toml::table& serverSection = TomlHelper::requiredTable(config, "server", CTX);
    int port = TomlHelper::requiredInt(serverSection, "port", CTX);
    std::string world = TomlHelper::requiredString(serverSection, "world", CTX);
    std::string map = TomlHelper::requiredString(serverSection, "map", CTX);
    
    const toml::table& combatSection = TomlHelper::requiredTable(config, "combat", CTX);
    float critProb = TomlHelper::requiredFloat(combatSection, "critical_probability", CTX);

    const toml::table& clansSection = TomlHelper::requiredTable(config, "clans", CTX);
    int clanRange = TomlHelper::requiredInt(clansSection, "bonus_range", CTX);
    float clanAttack = TomlHelper::requiredFloat(clansSection, "attack_bonus_per_member", CTX);
    float clanDefense = TomlHelper::requiredFloat(clansSection, "defense_bonus_per_member", CTX);

    return ServerConfig{
        port, world, map,
        critProb,
        clanRange, clanAttack, clanDefense
    };
}
