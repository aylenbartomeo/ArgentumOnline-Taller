#include "InteractionService.h"

InteractionResult InteractionService::startInteraction(uint32_t playerEntityId, Player& player,
                                                       Interactable* npc) {
    if (!npc) {
        return {InteractionStatus::FAILURE, "El NPC no existe."};
    }

    if (player.getPosition().chebyshev_distance_to(npc->getPosition()) > 2) {
        return {InteractionStatus::FAILURE, "El NPC está demasiado lejos."};
    }

    activeInteractions[playerEntityId] = npc;
    return npc->beInteractedBy(player);
}

InteractionResult InteractionService::executeCommand(uint32_t playerEntityId, Player& player,
                                                     const NpcCommandDTO& dto) {
    auto it = activeInteractions.find(playerEntityId);
    if (it == activeInteractions.end()) {
        return {InteractionStatus::FAILURE, "Debes seleccionar un NPC primero."};
    }

    Interactable* npc = it->second;

    if (player.getPosition().chebyshev_distance_to(npc->getPosition()) > 2) {
        activeInteractions.erase(playerEntityId);
        return {InteractionStatus::FAILURE, "Te has alejado demasiado del NPC."};
    }

    return npc->handleCommand(player, dto);
}

void InteractionService::endInteraction(uint32_t playerEntityId) {
    activeInteractions.erase(playerEntityId);
}

bool InteractionService::hasActiveInteraction(uint32_t playerEntityId) const {
    return activeInteractions.find(playerEntityId) != activeInteractions.end();
}
