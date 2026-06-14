#ifndef PLAYER_STATS_DTO_H
#define PLAYER_STATS_DTO_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct InventorySlotDTO {
    uint8_t slot{0};
    uint32_t itemId{0};
    uint16_t amount{0};
    bool isEquipped{false};
    std::string description;

    InventorySlotDTO() = default;
    InventorySlotDTO(uint8_t slot, uint32_t itemId, uint16_t amount, bool isEquipped,
                     std::string description = ""):
            slot(slot),
            itemId(itemId),
            amount(amount),
            isEquipped(isEquipped),
            description(std::move(description)) {}
};

struct PlayerStatsDTO {
    uint16_t currentHp = 0;
    uint16_t maxHp = 0;
    uint16_t currentMana = 0;
    uint16_t maxMana = 0;
    uint32_t gold = 0;
    uint32_t exp = 0;
    uint16_t level = 0;
    uint32_t expIntoLevel = 0;
    uint32_t expForLevel = 0;

    std::vector<InventorySlotDTO> inventory;

    PlayerStatsDTO() = default;
};

#endif  // PLAYER_STATS_DTO_H
