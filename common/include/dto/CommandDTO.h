#ifndef COMMAND_DTO_H
#define COMMAND_DTO_H

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#define MEDITATE_CMD "/meditar"
#define RESPAWN_CMD "/resucitar"
#define HEAL_CMD "/curar"
#define BUY_CMD "/comprar"
#define SELL_CMD "/vender"
#define DEPOSIT_CMD "/depositar"
#define WITHDRAW_CMD "/retirar"
#define TAKE_CMD "/tomar"
#define DROP_CMD "/tirar"

#define FOUND_CLAN_CMD "/fundar-clan"
#define JOIN_CLAN_CMD "/unirse-clan"
#define LEAVE_CLAN_CMD "/dejar-clan"
#define CHECK_CLAN_CMD "/revisar-clan"
#define ACCEPT_CLAN_CMD "/aceptar-clan"
#define REJECT_CLAN_CMD "/rechazar-clan"
#define KICK_CLAN_CMD "/clan-kick"

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

enum ActionItemType { TAKE, DROP, EQUIP, UNEQUIP };

struct ItemCommandDTO {
    ActionItemType action;
    uint8_t inventorySlot;
};

enum class Movement : uint8_t {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    DIAGONAL_UP_LEFT,
    DIAGONAL_UP_RIGHT,
    DIAGONAL_DOWN_LEFT,
    DIAGONAL_DOWN_RIGHT,
    STOP
};

enum interactCitizen { RESPAWN, HEAL, BUY, SELL, DEPOSIT, WITHDRAW };


#endif
