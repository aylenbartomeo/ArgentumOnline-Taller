#pragma once

#include "NpcCommandHandler.h"
#include "../entities/Player.h"

class ResurrectHandler : public NpcCommandHandler {
public:
    bool execute(Player& player, const NpcCommandDTO& dto) override {
        if (dto.type != NpcCommandType::RESPAWN) return false;

        if (!player.isDead()) return true;

        player.resurrect(); 
        return true;
    }
};
