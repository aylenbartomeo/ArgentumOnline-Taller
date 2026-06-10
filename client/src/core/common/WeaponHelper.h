#ifndef WEAPON_HELPER_H
#define WEAPON_HELPER_H

#include <algorithm>

#include "common/include/dto/PlayerStatsDTO.h"

namespace WeaponHelper {

constexpr uint32_t SWORD_WEAPON_ID = 2000;
constexpr uint32_t STAFF_WEAPON_ID = 203;
constexpr uint32_t VARA_FRESNO_WEAPON_ID = 2020;
constexpr uint32_t FLAUTA_WEAPON_ID = 2021;

inline bool hasEquipped(const PlayerStatsDTO& stats, uint32_t itemId) {
    return std::any_of(stats.inventory.begin(), stats.inventory.end(),
                       [itemId](const InventorySlotDTO& slot) {
                           return slot.isEquipped && slot.itemId == itemId;
                       });
}

inline bool hasSword(const PlayerStatsDTO& stats) { return hasEquipped(stats, SWORD_WEAPON_ID); }
inline bool hasStaff(const PlayerStatsDTO& stats) { return hasEquipped(stats, STAFF_WEAPON_ID); }
inline bool hasVaraFresno(const PlayerStatsDTO& stats) {
    return hasEquipped(stats, VARA_FRESNO_WEAPON_ID);
}
inline bool hasFlauta(const PlayerStatsDTO& stats) { return hasEquipped(stats, FLAUTA_WEAPON_ID); }

}  // namespace WeaponHelper

#endif
