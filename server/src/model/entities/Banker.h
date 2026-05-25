#ifndef BANKER_H
#define BANKER_H

#include <utility>
#include <functional>
#include "Player.h"
#include "../interfaces/Interactable.h"

class Banker : public Interactable {
public:
    void beInteractedBy(Player& player) override {
        player.setCurrentInteractable(this);
    }
};

#endif  // BANKER_H_
