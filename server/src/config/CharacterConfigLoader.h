#ifndef SERVER_SRC_CONFIG_CHARACTERCONFIGLOADER_H
#define SERVER_SRC_CONFIG_CHARACTERCONFIGLOADER_H

#include <filesystem>

#include "server/src/config/CharacterConfig.h"

class CharacterConfigLoader {
public:
    static CharacterConfigs loadCharacterConfigs(const std::filesystem::path& configPath);
};

#endif
