#include "ItemIcons.h"

#include "../core/common/WeaponHelper.h"

EquipSlot equipSlotForItem(uint32_t itemId) {
    if (itemId >= 1000 && itemId <= 1009) {
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
    if (itemId >= 4000 && itemId <= 4999) {
        if (itemId == 4002)
            return EquipSlot::SHIELD;
        return EquipSlot::WEAPON;
    }
    return EquipSlot::NONE;
}

const char* iconForItem(uint32_t itemId) {
    // --- CUERPO A CUERPO ---
    if (itemId == WeaponHelper::SWORD_WEAPON_ID)
        return "resources/items/espada.png";
    if (itemId == WeaponHelper::HACHA_WEAPON_ID)
        return "resources/items/hacha.png";
    if (itemId == WeaponHelper::MARTILLO_WEAPON_ID)
        return "resources/items/martillo.png";

    // --- ARMAS DE RANGO ---
    if (itemId == WeaponHelper::ARCO_SIMPLE_ID)
        return "resources/items/arco-simple.png";
    if (itemId == WeaponHelper::ARCO_COMPUESTO_ID)
        return "resources/items/arco-compuesto.png";
    if (itemId == WeaponHelper::VARA_FRESNO_WEAPON_ID)
        return "resources/items/vara-fresno.png";
    if (itemId == 2022)
        return "resources/items/baculo-nudoso.png";
    if (itemId == 2023)
        return "resources/items/baculo-engarzado-item.png";

    // --- BOSS DROPS ---
    if (itemId == 4001)
        return "resources/items/maza-de-titan-item.png";
    if (itemId == 4002)
        return "resources/items/escudo-infernal-item.png";
    if (itemId == 4003)
        return "resources/items/espada-de-magma-item.png";
    if (itemId == 4004)
        return "resources/items/arco-abismal-item.png";

    // --- ARMADURAS ---
    if (itemId == 1000)
        return "resources/items/armor/pechera-cuero.png";
    if (itemId == 1001)
        return "resources/items/armor/pechera-hierro.png";
    if (itemId == 1002)
        return "resources/items/armor/tunica.png";

    // --- ESCUDOS ---
    if (itemId == 1020)
        return "resources/items/armor/escudo-tortuga.png";
    if (itemId == 1021)
        return "resources/items/armor/escudo-hierro.png";

    // --- CASCOS Y SOMBREROS ---
    if (itemId == 1010)
        return "resources/items/armor/capucha.png";
    if (itemId == 1011)
        return "resources/items/armor/casco-hierro.png";
    if (itemId == 1012)
        return "resources/items/armor/sombrero-magico.png";

    switch (equipSlotForItem(itemId)) {
        case EquipSlot::WEAPON:
            return "resources/items/icon_weapon.png";
        case EquipSlot::ARMOR:
            return "resources/items/icon_armor.png";
        case EquipSlot::HELMET:
            return "resources/items/icon_helmet.png";
        case EquipSlot::SHIELD:
            return "resources/items/icon_shield.png";
        case EquipSlot::NONE:
            break;
    }

    if (itemId >= 3000 && itemId <= 3099) {
        return "resources/items/icon_potion.png";
    }
    return "resources/items/icon_unknown.png";
}
