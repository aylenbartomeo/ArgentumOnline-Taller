#ifndef SERVER_SRC_CONFIG_CHARACTERCONFIG_H
#define SERVER_SRC_CONFIG_CHARACTERCONFIG_H

#include "server/src/model/utils/types.h"

#include <cstdint>
#include <unordered_map>

struct PlayerConfig {
    int baseStrength;
    int baseIntelligence;
    int baseAgility;
    int baseConstitution;
    uint16_t startingLevel;
    uint32_t startingExperience;
    uint32_t startingGold;
};

struct RaceConfig {
    float lifeFactor;
    float manaFactor;
    float recoveryFactor;
};

struct CharacterClassConfig {
    float lifeFactor;
    float manaFactor;
    float meditationFactor;
    bool canUseMagic;
};

struct CharacterConfigs {
    PlayerConfig player;
    std::unordered_map<Race, RaceConfig> races;
    std::unordered_map<CharacterClass, CharacterClassConfig> classes;
};

#endif
