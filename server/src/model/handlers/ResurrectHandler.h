#pragma once

#include "../entities/Player.h"

#include "NpcCommandHandler.h"

class ResurrectHandler: public NpcCommandHandler {
public:
    InteractionResult execute(Player& player, const NpcCommandDTO& dto) override {
        InteractionResult result;

        // 1. Validar que el comando sea el correcto para este handler
        if (dto.type != NpcCommandType::RESPAWN) {
            result.status = InteractionStatus::UNHANDLED;
            return result;
        }

        // 2. Validar el estado del jugador (Regla de negocio: no podés revivir si ya estás vivo)
        if (!player.isDead()) {
            result.status = InteractionStatus::FAILURE;
            result.msg =
                    "¡Ya te encuentras con vida! No necesitas la bendición de la resurrección.";
            return result;
        }

        // 3. Ejecutar la acción en el modelo
        player.resurrect();

        // 4. Cargar el feedback para el World y el cliente
        result.status = InteractionStatus::SUCCESS;
        result.msg = "El Sacerdote posa sus manos sobre ti... ¡Has recuperado tu cuerpo físico!";
        return result;
    }
};
