#pragma once

#include "../../common/include/dto/ClientCommands.h"
#include "../../common/include/dto/CommandDTO.h"

class Player;

class NpcCommandHandler {
public:
    virtual ~NpcCommandHandler() = default;

    virtual InteractionResult execute(Player& player, const NpcCommandDTO& dto) = 0;
};
