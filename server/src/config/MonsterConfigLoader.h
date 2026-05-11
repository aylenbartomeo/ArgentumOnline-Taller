#ifndef SERVER_SRC_CONFIG_MONSTERCONFIGLOADER_H
#define SERVER_SRC_CONFIG_MONSTERCONFIGLOADER_H

#include "server/src/config/MonsterConfig.h"

#include <filesystem>

class MonsterConfigLoader {
public:
    static MonsterConfigs loadMonsterConfigs(const std::filesystem::path& configPath);
};

#endif
