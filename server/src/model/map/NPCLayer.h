#ifndef NPC_LAYER_H
#define NPC_LAYER_H

#include <cstdint>
#include <optional>
#include <vector>

#include "position.h"
#include "common/utils/types.h"

// Dato posicional de un NPC ciudadano en el mapa.
struct NPCSpawn {
    uint32_t id;
    NPCType type;       // MERCHANT, BANKER, PRIEST
    Position position;
};

class NPCLayer {
private:
    std::vector<NPCSpawn> npcSpawns;
    uint32_t nextNpcId = 1;

public:
    NPCLayer() = default;
    ~NPCLayer() = default;

    uint32_t addNPC(NPCType type, const Position& pos);
    std::optional<NPCSpawn> findNPCAt(const Position& pos) const;
    std::vector<NPCSpawn> findNPCsInRange(const Position& center, int range) const;
    const std::vector<NPCSpawn>& getAllNPCs() const;
    void clear();
};

#endif // NPC_LAYER_H
