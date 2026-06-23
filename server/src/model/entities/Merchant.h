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
    uint32_t id;
    Position pos;
    std::unordered_map<uint32_t, int> stock;
    std::unordered_map<NpcCommandType, std::unique_ptr<NpcCommandHandler>> commandHandlers;

public:
    Merchant(uint32_t id, Position pos, const ItemRegistry& registry,
             std::unordered_map<uint32_t, int> initialStock);

    Position getPosition() const override { return pos; }
    uint32_t getId() const override { return id; }
    const std::unordered_map<uint32_t, int>& getStock() const { return stock; }
    void setStock(const std::unordered_map<uint32_t, int>& savedStock) { stock = savedStock; }
    InteractionResult beInteractedBy(Player& player) override;
    InteractionResult handleCommand(Player& player, const NpcCommandDTO& dto) override;
};
