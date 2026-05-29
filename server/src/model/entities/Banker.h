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
    uint32_t id;
    Position pos;
    std::unordered_map<NpcCommandType, std::unique_ptr<NpcCommandHandler>> commandHandlers;

public:
    Banker(uint32_t id, Position pos, GlobalBank& bankInstance, const ItemRegistry& registry);
    Position getPosition() const override { return pos; }
    uint32_t getId() const override { return id; }
    InteractionResult beInteractedBy(Player& player) override;
    InteractionResult handleCommand(Player& player, const NpcCommandDTO& dto) override;
};
