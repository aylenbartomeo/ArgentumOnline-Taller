#ifndef SERVER_SRC_CONFIG_INVENTORYCONFIG_H
#define SERVER_SRC_CONFIG_INVENTORYCONFIG_H

#include <cstdint>

struct InventoryConfig {
    uint8_t maxSlots;
    uint32_t gold;
    uint32_t maxGold;
    uint32_t initialSafeGold;
};

#endif  // SERVER_SRC_CONFIG_INVENTORYCONFIG_H
