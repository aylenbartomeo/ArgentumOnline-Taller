#ifndef SERVER_SRC_CONFIG_INVENTORYCONFIG_H
#define SERVER_SRC_CONFIG_INVENTORYCONFIG_H

#include <cstdint>

struct InventoryConfig {
    uint8_t maxSlots;
    uint32_t maxGold;
};

#endif  // SERVER_SRC_CONFIG_INVENTORYCONFIG_H
