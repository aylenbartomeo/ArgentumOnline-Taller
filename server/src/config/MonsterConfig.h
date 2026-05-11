#ifndef SERVER_SRC_CONFIG_MONSTERCONFIG_H
#define SERVER_SRC_CONFIG_MONSTERCONFIG_H

#include <string>
#include <unordered_map>

#include "server/src/model/utils/types.h"

struct MonsterConfig {
    int maxHealth;
    int strength;
    int agility;
    int attackMin;
    int attackMax;
    int detectionRange;
    int attackRange;
    std::string zone;
};

using MonsterConfigs = std::unordered_map<NPCType, MonsterConfig>;

#endif
