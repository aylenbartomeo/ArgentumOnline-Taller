#include "server/src/config/TomlConfigHelper.h"

struct ServerConfig {
    int port;
    std::string worldName;
    std::string mapPath;
};

ServerConfig loadServerConfig(const std::filesystem::path& path) {
    constexpr std::string_view CTX = "server_system";
    
    toml::table config = TomlHelper::parseConfigFile(path, CTX);
    const toml::table& serverSection = TomlHelper::requiredTable(config, "server", CTX);
    
    return ServerConfig{
        TomlHelper::requiredInt(serverSection, "port", CTX),
        TomlHelper::requiredString(serverSection, "world", CTX),
        TomlHelper::requiredString(serverSection, "map", CTX)
    };
}
