#ifndef SERVER_SRC_CONFIG_MONSTERCONFIGLOADER_H
#define SERVER_SRC_CONFIG_MONSTERCONFIGLOADER_H

#include <filesystem>

#include "server/src/config/MonsterConfig.h"

class MonsterConfigLoader {
public:
    static MonsterConfigs loadMonsterConfigs(const std::filesystem::path& configPath);
};

#endif
