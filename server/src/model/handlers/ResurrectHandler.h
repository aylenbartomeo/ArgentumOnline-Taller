#pragma once

#include "../entities/Player.h"

#include "NpcCommandHandler.h"

class ResurrectHandler: public NpcCommandHandler {
public:
    bool execute(Player& player, const NpcCommandDTO& dto) override {
        if (dto.type != NpcCommandType::RESPAWN)
            return false;

        if (!player.isDead())
            return true;

        player.resurrect();
        return true;
    }
};
