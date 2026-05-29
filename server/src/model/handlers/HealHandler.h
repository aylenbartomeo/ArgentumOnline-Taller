#pragma once

#include "../entities/Player.h"

#include "NpcCommandHandler.h"

class HealHandler : public NpcCommandHandler {
public:
    InteractionResult execute(Player& player, const NpcCommandDTO& dto) override {
        InteractionResult result;

        // 1. Si no es mi comando, aviso explícitamente al World que no lo manejé
        if (dto.type != NpcCommandType::HEAL) {
            result.status = InteractionStatus::UNHANDLED;
            return result; 
        }

        // A partir de acá, el comando SÍ es mío. El estatus base pasa a ser de error
        // a menos que se completen todas las condiciones.
        result.status = InteractionStatus::FAILURE;

        if (player.isDead()) {
            result.msg = "Los fantasmas no pueden ser curados. Resucita primero.";
            return result;
        }

        if (player.getHp() == player.getMaxHp() && player.getMana() == player.getMaxMana()) {
            result.msg = "Ya te encuentras desbordando de salud y energía mágica.";
            return result;
        }

        // Todo salió perfecto
        player.restoreHp();
        player.restoreMana();

        result.status = InteractionStatus::SUCCESS;
        result.msg = "El Sacerdote murmura una plegaria... ¡Tus heridas sanan!";
        return result;
    }
};
