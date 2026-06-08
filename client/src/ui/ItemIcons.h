#ifndef ITEM_ICONS_H
#define ITEM_ICONS_H

#include <cstdint>

enum class EquipSlot { NONE, WEAPON, ARMOR, HELMET, SHIELD };

EquipSlot equipSlotForItem(uint32_t itemId);
const char* iconForItem(uint32_t itemId);

#endif
