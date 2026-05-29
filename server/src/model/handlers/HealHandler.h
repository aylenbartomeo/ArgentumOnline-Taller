#pragma once

#include "../entities/Player.h"

#include "NpcCommandHandler.h"

class HealHandler: public NpcCommandHandler {
public:
    bool execute(Player& player, const NpcCommandDTO& dto) override {
        if (dto.type != NpcCommandType::HEAL)
            return false;

        if (player.isDead())
            return true;

        if (player.getHp() == player.getMaxHp() && player.getMana() == player.getMaxMana()) {
            return true;
        }

        player.restoreHp();
        player.restoreMana();
        return true;
    }
};
