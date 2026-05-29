#ifndef PRIEST_H
#define PRIEST_H

#include <memory>
#include <unordered_map>

#include "../handlers/NpcCommandHandler.h"
#include "../interfaces/Interactable.h"

#include "Player.h"

class Priest: public Interactable {
private:
    uint32_t id;
    Position pos;
    std::unordered_map<uint32_t, int> stock;
    std::unordered_map<NpcCommandType, std::unique_ptr<NpcCommandHandler>> commandHandlers;

public:
    Priest(uint32_t id, Position pos, const ItemRegistry& registry);
    Position getPosition() const override { return pos; }
    uint32_t getId() const override { return id; }
    void beInteractedBy(Player& player) override;
    void handleCommand(Player& player, const NpcCommandDTO& dto) override;
};

#endif  // PRIEST_H_
