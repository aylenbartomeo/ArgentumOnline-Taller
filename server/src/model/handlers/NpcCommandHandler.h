#pragma once

#include "../../common/include/dto/ClientCommands.h"
#include "../../common/include/dto/CommandDTO.h"

class Player;

class NpcCommandHandler {
public:
    virtual ~NpcCommandHandler() = default;

    // Ejecuta la lógica específica. Retorna true si procesó el comando con éxito.
    virtual bool execute(Player& player, const NpcCommandDTO& dto) = 0;
};
