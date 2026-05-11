#ifndef SERVER_SRC_CONFIG_CHARACTERCONFIGLOADER_H
#define SERVER_SRC_CONFIG_CHARACTERCONFIGLOADER_H

#include "server/src/config/CharacterConfig.h"

#include <filesystem>

class CharacterConfigLoader {
public:
    static CharacterConfigs loadCharacterConfigs(const std::filesystem::path& configPath);
};

#endif
