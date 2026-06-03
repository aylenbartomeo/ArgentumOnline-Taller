#ifndef COMMAND_DTO_H
#define COMMAND_DTO_H

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#define MEDITATE_CMD "/meditar"
#define TAKE_CMD "/tomar"
#define DROP_CMD "/tirar"
#define RESPAWN_CMD "/resucitar"
#define HEAL_CMD "/curar"
#define BUY_CMD "/comprar"
#define SELL_CMD "/vender"
#define DEPOSIT_CMD "/depositar"
#define WITHDRAW_CMD "/retirar"

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
    STOP
};

enum class InteractionStatus {
    UNHANDLED,  // El handler no reconoció el comando (No era para él)
    SUCCESS,    // Se ejecutó con éxito
    FAILURE     // El comando era para él, pero falló una regla de negocio (ej: sin oro)
};

struct InteractionResult {
    InteractionStatus status{InteractionStatus::UNHANDLED};
    std::string msg;
};

#endif
