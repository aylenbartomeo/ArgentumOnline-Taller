#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "../handlers/NpcCommandHandler.h"
#include "../interfaces/Interactable.h"

class Item;
class ItemRegistry;

class Merchant: public Interactable {
private:
    Position pos;
    std::unordered_map<uint32_t, int> stock;
    std::unordered_map<NpcCommandType, std::unique_ptr<NpcCommandHandler>> commandHandlers;

public:
    Merchant(Position pos, const ItemRegistry& registry);

    Position getPosition() const override { return pos; }
    void beInteractedBy(Player& player) override;
    void handleCommand(Player& player, const NpcCommandDTO& dto) override;
};
