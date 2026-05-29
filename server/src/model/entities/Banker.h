#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

#include "../handlers/NpcCommandHandler.h"
#include "../interfaces/Interactable.h"

#include "Player.h"

class GlobalBank;
class ItemRegistry;

class Banker: public Interactable {
private:
    Position pos;
    std::unordered_map<NpcCommandType, std::unique_ptr<NpcCommandHandler>> commandHandlers;

public:
    Banker(Position pos, GlobalBank& bankInstance, const ItemRegistry& registry);
    Position getPosition() const override { return pos; }
    void beInteractedBy(Player& player) override;
    void handleCommand(Player& player, const NpcCommandDTO& dto) override;
};
