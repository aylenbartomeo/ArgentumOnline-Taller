#ifndef PLAYER_STATS_DTO_H
#define PLAYER_STATS_DTO_H

#include <cstdint>
#include <vector>

struct InventorySlotDTO {
    uint8_t slot{0};
    uint32_t itemId{0};
    uint16_t amount{0};
    bool isEquipped{false};

    InventorySlotDTO() = default;
    InventorySlotDTO(uint8_t slot, uint32_t itemId, uint16_t amount, bool isEquipped):
            slot(slot), itemId(itemId), amount(amount), isEquipped(isEquipped) {}
};

struct PlayerStatsDTO {
    uint16_t currentHp = 0;
    uint16_t maxHp = 0;
    uint16_t currentMana = 0;
    uint16_t maxMana = 0;
    uint32_t gold = 0;
    uint32_t exp = 0;
    uint16_t level = 0;

    std::vector<InventorySlotDTO> inventory;

    PlayerStatsDTO() = default;
};

#endif  // PLAYER_STATS_DTO_H
