#pragma once

#include "../interfaces/Interactable.h"
#include <unordered_map>
#include <string>
#include <memory>

class Item;
class ItemRegistry;

class Merchant : public Interactable {
private:
    Position position;
    const ItemRegistry& registry;  // catálogo global — no vende hechizos
    // Stock propio: lo que los jugadores le vendieron
    std::unordered_map<uint32_t, int> stock;
public:
    Merchant(Position pos, const ItemRegistry& registry);
    Position getPosition() const override { return position; }
    void beInteractedBy(Player& player, std::vector<WorldEvent>& outgoingEvents) override;
    void handleCommand(Player& player, const NpcCommandDTO& dto, std::vector<WorldEvent>& outgoingEvents) override;
};
