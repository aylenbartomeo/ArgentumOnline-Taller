#ifndef ITEMACCIONDTO_H
#define ITEMACCIONDTO_H

#include <cstdint>

enum class ActionItemType : uint8_t {
    TAKE,
    DROP,
    EQUIP,
    UNEQUIP
};

struct ItemCommandDTO {
    ActionItemType action;
    uint8_t inventorySlot;
};

#endif