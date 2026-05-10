#ifndef INVENTORYSTATUS_H
#define INVENTORYSTATUS_H

#include <cstdint>
#include <vector>

struct ItemInfoDTO {
    uint32_t itemId;
    uint16_t quantity;
    bool isEquipped;
};

struct InventoryStateDTO {
    std::vector<ItemInfoDTO> items;
    uint32_t safeGold;
    uint32_t excessGold;

    uint32_t equippedWeaponId;
    uint32_t equippedArmorId;
    uint32_t equippedHelmetId;
    uint32_t equippedShieldId;
};

#endif