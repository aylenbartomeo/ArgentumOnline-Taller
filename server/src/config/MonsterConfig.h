#ifndef MONSTERCONFIG_H
#define MONSTERCONFIG_H

#include <string>
#include <unordered_map>

#include "../../common/utils/types.h"

struct MonsterConfig {
    int maxHealth;
    int strength;
    int agility;
    int attackMin;
    int attackMax;
    int detectionRange;
    int attackRange;
    int level;
    std::string zone;
    int attackCooldownMs;
    int moveCooldownMs;
};

using MonsterConfigs = std::unordered_map<NPCType, MonsterConfig>;

#endif
