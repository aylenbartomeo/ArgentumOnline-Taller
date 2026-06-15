#ifndef WEAPON_HELPER_H
#define WEAPON_HELPER_H

#include <algorithm>

#include "common/include/dto/PlayerStatsDTO.h"

namespace WeaponHelper {

constexpr uint32_t SWORD_WEAPON_ID = 2000;
constexpr uint32_t HACHA_WEAPON_ID = 2001;
constexpr uint32_t MARTILLO_WEAPON_ID = 2002;
constexpr uint32_t STAFF_WEAPON_ID = 203;
constexpr uint32_t VARA_FRESNO_WEAPON_ID = 2020;
constexpr uint32_t FLAUTA_WEAPON_ID = 2021;
constexpr uint32_t ARCO_SIMPLE_ID = 2010;
constexpr uint32_t ARCO_COMPUESTO_ID = 2011;

inline bool hasEquipped(const PlayerStatsDTO& stats, uint32_t itemId) {
    return std::any_of(stats.inventory.begin(), stats.inventory.end(),
                       [itemId](const InventorySlotDTO& slot) {
                           return slot.isEquipped && slot.itemId == itemId;
                       });
}

inline bool hasSword(const PlayerStatsDTO& stats) { return hasEquipped(stats, SWORD_WEAPON_ID); }
inline bool hasAxe(const PlayerStatsDTO& stats) { return hasEquipped(stats, HACHA_WEAPON_ID); }
inline bool hasHammer(const PlayerStatsDTO& stats) {
    return hasEquipped(stats, MARTILLO_WEAPON_ID);
}
inline bool hasStaff(const PlayerStatsDTO& stats) { return hasEquipped(stats, STAFF_WEAPON_ID); }
inline bool hasVaraFresno(const PlayerStatsDTO& stats) {
    return hasEquipped(stats, VARA_FRESNO_WEAPON_ID);
}
inline bool hasFlauta(const PlayerStatsDTO& stats) { return hasEquipped(stats, FLAUTA_WEAPON_ID); }
inline bool hasBow(const PlayerStatsDTO& stats) {
    return hasEquipped(stats, ARCO_SIMPLE_ID) || hasEquipped(stats, ARCO_COMPUESTO_ID);
}

}  // namespace WeaponHelper

#endif
