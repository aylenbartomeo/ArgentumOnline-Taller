#pragma once

#include "NpcCommandHandler.h"
#include "../entities/Player.h"

class HealHandler : public NpcCommandHandler {
public:
    bool execute(Player& player, const NpcCommandDTO& dto) override {
        if (dto.type != NpcCommandType::HEAL) return false;

        if (player.isDead()) return true;

        if (player.getHp() == player.getMaxHp() && player.getMana() == player.getMaxMana()) {
            return true;
        }

        player.restoreHp();
        player.restoreMana();
        return true;
    }
};
