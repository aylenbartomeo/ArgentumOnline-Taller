#ifndef INTERACTION_SERVICE_H
#define INTERACTION_SERVICE_H

#include <unordered_map>
#include <cstdint>
#include "model/entities/Player.h"
#include "model/interfaces/Interactable.h"

class InteractionService {
private:
    std::unordered_map<uint32_t, Interactable*> activeInteractions;

public:
    InteractionResult startInteraction(uint32_t playerEntityId, Player& player, Interactable* npc);
    InteractionResult executeCommand(uint32_t playerEntityId, Player& player, const NpcCommandDTO& dto);
    void endInteraction(uint32_t playerEntityId);
    bool hasActiveInteraction(uint32_t playerEntityId) const;
};

#endif // INTERACTION_SERVICE_H
