#include "NPCLayer.h"

uint32_t NPCLayer::addNPC(NPCType type, const Position& pos) {
    uint32_t id = nextNpcId++;
    npcSpawns.push_back(NPCSpawn{id, type, pos});
    return id;
}

std::optional<NPCSpawn> NPCLayer::findNPCAt(const Position& pos) const {
    for (const auto& npc : npcSpawns) {
        if (npc.position == pos) {
            return npc;
        }
    }
    return std::nullopt;
}

std::vector<NPCSpawn> NPCLayer::findNPCsInRange(const Position& center, int range) const {
    std::vector<NPCSpawn> inRange;
    for (const auto& npc : npcSpawns) {
        if (npc.position.chebyshev_distance_to(center) <= range) {
            inRange.push_back(npc);
        }
    }
    return inRange;
}

const std::vector<NPCSpawn>& NPCLayer::getAllNPCs() const {
    return npcSpawns;
}

void NPCLayer::clear() {
    npcSpawns.clear();
    nextNpcId = 1;
}
