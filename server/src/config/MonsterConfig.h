#ifndef MONSTERCONFIG_H
#define MONSTERCONFIG_H

#include <string>
#include <unordered_map>
#include <vector>

#include "../../common/utils/types.h"

struct MonsterConfig {
    int maxHealth;
    int strength;
    int agility;
    int attackMin;
    int attackMax;
    int detectionRange;
    int attackRange;
    int minLevel;
    int maxLevel;
    std::string zone;
    int attackCooldownMs;
    int moveCooldownMs;
    bool isBoss = false;
    uint32_t uniqueDropItemId = 0;
    uint32_t guaranteedGold = 0;
    std::vector<uint32_t> extraLoot;
};

using MonsterConfigs = std::unordered_map<NPCType, MonsterConfig>;

#endif
