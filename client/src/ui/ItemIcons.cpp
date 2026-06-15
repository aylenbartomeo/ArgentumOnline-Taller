#include "ItemIcons.h"

#include "../core/common/WeaponHelper.h"

EquipSlot equipSlotForItem(uint32_t itemId) {
    if (itemId >= 1001 && itemId <= 1009) {
        return EquipSlot::ARMOR;
    }
    if (itemId >= 1010 && itemId <= 1019) {
        return EquipSlot::HELMET;
    }
    if (itemId >= 1020 && itemId <= 1029) {
        return EquipSlot::SHIELD;
    }
    if (itemId >= 2000 && itemId <= 2999) {
        return EquipSlot::WEAPON;
    }
    return EquipSlot::NONE;
}

const char* iconForItem(uint32_t itemId) {
    if (itemId == WeaponHelper::SWORD_WEAPON_ID)
        return "resources/items/espada-item.png";
    if (itemId == WeaponHelper::HACHA_WEAPON_ID)
        return "resources/items/hacha-item.png";
    if (itemId == WeaponHelper::MARTILLO_WEAPON_ID)
        return "resources/items/martillo-item.png";
    switch (equipSlotForItem(itemId)) {
        case EquipSlot::WEAPON:
            return "resources/icon_weapon.png";
        case EquipSlot::ARMOR:
            return "resources/icon_armor.png";
        case EquipSlot::HELMET:
            return "resources/icon_helmet.png";
        case EquipSlot::SHIELD:
            return "resources/icon_shield.png";
        case EquipSlot::NONE:
            break;
    }
    if (itemId >= 3000 && itemId <= 3099) {
        return "resources/icon_potion.png";
    }
    return "resources/icon_unknown.png";
}
