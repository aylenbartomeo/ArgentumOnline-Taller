#ifndef BANKER_H
#define BANKER_H

#include <utility>
#include <functional>
#include "Player.h"
#include "../interfaces/Interactable.h"
#include "GlobalBank.h"

class Banker : public Interactable {
private:
    GlobalBank& bank;
public:
    void beInteractedBy(Player& player, std::vector<WorldEvent>& outgoingEvents) override {}
    void handleCommand(Player& player, const NpcCommandDTO& dto, std::vector<WorldEvent>& outgoingEvents) override {}
};

#endif  // BANKER_H_
