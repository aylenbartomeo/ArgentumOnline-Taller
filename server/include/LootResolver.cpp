#include "LootResolver.h"

#include "FormulaEngine.h"
#include "RandomNumberGenerator.h"

NpcLootResult LootResolver::resolveNpcLoot(uint16_t npcMaxLife,
                                           const std::vector<int>& potionIds,
                                           const std::vector<int>& allDroppableItemIds) {
    NpcLootResult result;
    RandomNumberGenerator rng;
    float roll = rng(0.0f, 1.0f);

    if (roll < 0.90f) {
        // Nada (0.80 original + 0.10 restante)
        return result;
    } else if (roll < 0.98f) {
        // Oro (0.08)
        result.dropsGold = true;
        result.goldAmount = FormulaEngine::getInstance().calculate_npc_gold_drop(npcMaxLife);
        return result;
    } else if (roll < 0.99f) {
        // Poción random (0.01)
        if (!potionIds.empty()) {
            int idx = rng(0, static_cast<int>(potionIds.size()) - 1);
            result.dropsItem = true;
            result.droppedItemId = potionIds[idx];
        }
        return result;
    } else {
        // Item random (0.01)
        if (!allDroppableItemIds.empty()) {
            int idx = rng(0, static_cast<int>(allDroppableItemIds.size()) - 1);
            result.dropsItem = true;
            result.droppedItemId = allDroppableItemIds[idx];
        }
        return result;
    }
}
