#ifndef PRIEST_H
#define PRIEST_H

#include "Player.h"
#include "../interfaces/Interactable.h"

class Priest : public Interactable {
private:
    const ItemRegistry& registry; // Solo vende hechizos
public:
    void beInteractedBy(Player& player, std::vector<WorldEvent>& outgoingEvents) override {}
    void handleCommand(Player& player, const NpcCommandDTO& dto, std::vector<WorldEvent>& outgoingEvents) override {}
};

#endif  // PRIEST_H_
