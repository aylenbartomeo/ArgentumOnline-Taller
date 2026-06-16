#include "LootResolver.h"

#include "FormulaEngine.h"
#include "RandomNumberGenerator.h"

NpcLootResult LootResolver::resolveNpcLoot(uint16_t npcMaxLife, const std::vector<int>& potionIds,
                                           const std::vector<int>& allDroppableItemIds) {
    NpcLootResult result;

    static RandomNumberGenerator rng;

    float roll = rng(0.0f, 0.90f);

    if (roll < 0.80f) {
        // Nada: ocupa exactamente el 80% del espacio probabilístico
        return result;
    } else if (roll < 0.88f) {
        // Oro: ocupa exactamente el 8% (0.80 a 0.88)
        result.dropsGold = true;
        result.goldAmount = FormulaEngine::getInstance().calculate_npc_gold_drop(npcMaxLife);
        return result;
    } else if (roll < 0.89f) {
        // Poción random: ocupa exactamente el 1% (0.88 a 0.89)
        if (!potionIds.empty()) {
            int idx = rng(0, static_cast<int>(potionIds.size()) - 1);
            result.dropsItem = true;
            result.droppedItemId = potionIds[idx];
        }
        return result;
    } else {
        // Item random: ocupa exactamente el 1% (0.89 a 0.90)
        if (!allDroppableItemIds.empty()) {
            int idx = rng(0, static_cast<int>(allDroppableItemIds.size()) - 1);
            result.dropsItem = true;
            result.droppedItemId = allDroppableItemIds[idx];
        }
        return result;
    }
}
