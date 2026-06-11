#include "StateAudioTrigger.h"

#include <algorithm>
#include <numeric>
#include <vector>

bool StateAudioTrigger::checkAndTrigger(const PlayerStatsDTO& oldStats,
                                        const PlayerStatsDTO& newStats, AudioSystem& audio) {
    bool wasDead = (oldStats.maxHp > 0 && oldStats.currentHp <= 0);
    bool isAliveNow = (newStats.maxHp > 0 && newStats.currentHp > 0);

    if (wasDead && isAliveNow)
        audio.playSound(SoundEffect::RESURRECT);
    if (newStats.gold > oldStats.gold)
        audio.playSound(SoundEffect::PICK_GOLD);

    bool wasAlive = (oldStats.maxHp > 0 && oldStats.currentHp > 0);
    bool isDeadNow = (newStats.maxHp > 0 && newStats.currentHp <= 0);

    uint32_t oldItemCount =
            std::accumulate(oldStats.inventory.begin(), oldStats.inventory.end(), uint32_t{0},
                            [](uint32_t sum, const auto& slot) { return sum + slot.amount; });
    uint32_t newItemCount =
            std::accumulate(newStats.inventory.begin(), newStats.inventory.end(), uint32_t{0},
                            [](uint32_t sum, const auto& slot) { return sum + slot.amount; });

    if (newItemCount > oldItemCount)
        audio.playSound(SoundEffect::PICK_ITEM);
    else if (newItemCount < oldItemCount)
        audio.playSound(SoundEffect::DROP_ITEM);

    if (oldStats.level > 0 && newStats.level > oldStats.level) {
        audio.playSound(SoundEffect::LEVEL_UP);
    }

    if (wasAlive && isDeadNow) {
        audio.playSound(SoundEffect::DIE);
    }

    std::vector<uint8_t> oldEquipped, newEquipped;
    for (const auto& slot: oldStats.inventory)
        if (slot.isEquipped)
            oldEquipped.push_back(slot.slot);
    for (const auto& slot: newStats.inventory)
        if (slot.isEquipped)
            newEquipped.push_back(slot.slot);
    std::sort(oldEquipped.begin(), oldEquipped.end());
    std::sort(newEquipped.begin(), newEquipped.end());

    if (oldEquipped != newEquipped)
        audio.playSound(SoundEffect::EQUIP_WEAPON);

    bool tookDamage = wasAlive && !isDeadNow && (oldStats.maxHp > 0) &&
                      (newStats.currentHp < oldStats.currentHp);
    return tookDamage;
}
