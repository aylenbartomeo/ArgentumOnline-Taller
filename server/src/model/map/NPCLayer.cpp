#include "NPCLayer.h"

#include <algorithm>

uint32_t NPCLayer::addNPC(NPCType type, const Position& pos) {
    uint32_t id = nextNpcId++;
    npcSpawns.push_back(NPCSpawn{id, type, pos});
    return id;
}

std::optional<NPCSpawn> NPCLayer::findNPCAt(const Position& pos) const {
    auto it = std::find_if(npcSpawns.begin(), npcSpawns.end(),
                           [&pos](const NPCSpawn& npc) { return npc.position == pos; });
    if (it != npcSpawns.end()) {
        return *it;
    }
    return std::nullopt;
}

std::vector<NPCSpawn> NPCLayer::findNPCsInRange(const Position& center, int range) const {
    std::vector<NPCSpawn> inRange;
    std::copy_if(npcSpawns.begin(), npcSpawns.end(), std::back_inserter(inRange),
                 [&center, range](const NPCSpawn& npc) {
                     return npc.position.chebyshev_distance_to(center) <= range;
                 });
    return inRange;
}

const std::vector<NPCSpawn>& NPCLayer::getAllNPCs() const { return npcSpawns; }

void NPCLayer::clear() {
    npcSpawns.clear();
    nextNpcId = 1;
}
