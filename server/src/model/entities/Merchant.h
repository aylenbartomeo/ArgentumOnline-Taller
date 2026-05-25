#ifndef MERCHANT_H
#define MERCHANT_H

#include "Player.h"
#include "../interfaces/Interactable.h"

class Merchant : public Interactable {
public:
    void beInteractedBy(Player& player) override {
        player.setCurrentInteractable(this);
    }
};

#endif  // MERCHANT_H
