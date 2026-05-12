#ifndef INVENTORYCONFIGLOADER_H
#define INVENTORYCONFIGLOADER_H

#include <filesystem>

#include "server/src/config/InventoryConfig.h"

class InventoryConfigLoader {
public:
    static InventoryConfig loadInventoryConfig(const std::filesystem::path& configPath);
};

#endif  // INVENTORYCONFIGLOADER_H
