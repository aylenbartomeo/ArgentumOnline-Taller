#ifndef BANKER_H
#define BANKER_H

#include <utility>
#include <functional>
#include "Player.h"
#include "../interfaces/Interactable.h"
#include "GlobalBank.h"

class ItemRegistry;

class Banker : public Interactable {
private:
    Position pos;
    GlobalBank& bank;
    const ItemRegistry& registry;
public:
    Banker(Position pos, GlobalBank& bankInstance, const ItemRegistry& registry);
    Position getPosition() const override { return pos; }
    void beInteractedBy(Player& player) override;
    void handleCommand(Player& player, const NpcCommandDTO& dto) override;
};

#endif  // BANKER_H_
