#ifndef PRIEST_H
#define PRIEST_H

#include "Player.h"
#include "../interfaces/Interactable.h"

class ItemRegistry;

class Priest : public Interactable {
private:
    Position pos;
    const ItemRegistry& registry; // Solo vende hechizos
public:
    Priest(Position pos, const ItemRegistry& registry);
    Position getPosition() const override { return pos; }
    void beInteractedBy(Player& player) override;
    void handleCommand(Player& player, const NpcCommandDTO& dto) override;
};

#endif  // PRIEST_H_
