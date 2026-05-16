#ifndef CHARACTERCONFIGLOADER_H
#define CHARACTERCONFIGLOADER_H

#include <filesystem>

#include "CharacterConfig.h"

class CharacterConfigLoader {
public:
    static CharacterConfigs loadCharacterConfigs(const std::filesystem::path& configPath);
};

#endif
