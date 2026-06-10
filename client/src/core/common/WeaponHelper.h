#ifndef WEAPON_HELPER_H
#define WEAPON_HELPER_H

#include <algorithm>

#include "../common/GameConstants.h"
#include "common/include/dto/PlayerStatsDTO.h"

namespace WeaponHelper {

inline bool hasEquipped(const PlayerStatsDTO& stats, uint32_t itemId) {
    return std::any_of(stats.inventory.begin(), stats.inventory.end(),
                       [itemId](const InventorySlotDTO& slot) {
                           return slot.isEquipped && slot.itemId == itemId;
                       });
}

inline bool hasSword(const PlayerStatsDTO& stats) {
    return hasEquipped(stats, GameConstants::SWORD_WEAPON_ID);
}
inline bool hasStaff(const PlayerStatsDTO& stats) {
    return hasEquipped(stats, GameConstants::STAFF_WEAPON_ID);
}
inline bool hasVaraFresno(const PlayerStatsDTO& stats) {
    return hasEquipped(stats, GameConstants::VARA_FRESNO_WEAPON_ID);
}
inline bool hasFlauta(const PlayerStatsDTO& stats) {
    return hasEquipped(stats, GameConstants::FLAUTA_WEAPON_ID);
}

}  // namespace WeaponHelper

#endif
