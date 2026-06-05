#ifndef LOOT_RESOLVER_H_
#define LOOT_RESOLVER_H_

#include <cstdint>
#include <vector>

struct NpcLootResult {
    bool dropsGold = false;
    uint32_t goldAmount = 0;
    bool dropsItem = false;
    uint32_t droppedItemId = 0;
};

class LootResolver {
public:
    // Resuelve el loot de un NPC muerto según las probabilidades del juego.
    // Utiliza FormulaEngine internamente para calcular la cantidad de oro.
    static NpcLootResult resolveNpcLoot(uint16_t npcMaxLife, const std::vector<int>& potionIds,
                                        const std::vector<int>& allDroppableItemIds);
};

#endif  // LOOT_RESOLVER_H_
