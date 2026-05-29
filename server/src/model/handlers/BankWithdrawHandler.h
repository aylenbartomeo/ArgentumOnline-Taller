#pragma once

#include <sstream>
#include <string>

#include "../entities/GlobalBank.h"
#include "../entities/Player.h"
#include "../items/ItemRegistry.h"

#include "NpcCommandHandler.h"

class BankWithdrawHandler: public NpcCommandHandler {
private:
    GlobalBank& bankInstance;

public:
    explicit BankWithdrawHandler(GlobalBank& bankInstance): bankInstance(bankInstance) {}

    InteractionResult execute(Player& player, const NpcCommandDTO& dto) override {
        InteractionResult result;

        // 1. Validar si el comando corresponde a este handler
        if (dto.type != NpcCommandType::WITHDRAW) {
            result.status = InteractionStatus::UNHANDLED;
            return result;
        }

        // 2. Regla de negocio: Los fantasmas no tienen bolsillos físicos
        if (player.isDead()) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "Los fantasmas no pueden retirar bienes de sus cuentas bancarias.";
            return result;
        }

        uint32_t playerId = player.getDbId();
        std::stringstream ss(dto.arg);
        std::string subComando;
        ss >> subComando;

        if (subComando == "oro") {
            uint32_t amount = 0;
            if (!(ss >> amount) || amount == 0) {
                result.status = InteractionStatus::FAILURE;
                result.msg = "Cantidad de oro inválida. Uso: /retirar oro <cantidad>";
                return result;
            }

            if (!bankInstance.withdrawGold(playerId, amount)) {
                result.status = InteractionStatus::FAILURE;
                result.msg = "No posees esa cantidad de oro depositada en tu cuenta.";
                return result;
            }

            player.addGold(amount);
            result.status = InteractionStatus::SUCCESS;
            result.msg =
                    "Retiro exitoso: Ha retirado " + std::to_string(amount) + " monedas de oro.";
            return result;
        }

        // En lugar de usar dto.arg completo, usamos 'subComando' que ya tiene la primera palabra
        // aislada
        uint32_t itemId = 0;
        try {
            itemId = std::stoul(subComando);
        } catch (...) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "Argumento inválido. Uso: /retirar oro <cant> o /retirar <itemId>";
            return result;
        }

        // Intentamos extraer una unidad del banco
        uint16_t withdrawnAmount = bankInstance.withdrawItemById(playerId, itemId, 1);
        if (withdrawnAmount == 0) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "No posees ese artículo en tu bóveda bancaria.";
            return result;
        }

        // Intentamos meterlo en el inventario del jugador
        if (!player.addItem(itemId, 1)) {
            // Rollback atómico si la mochila está colmada
            bankInstance.depositItem(playerId, itemId, 1);
            result.status = InteractionStatus::FAILURE;
            result.msg = "Tu inventario está lleno. El artículo permanece seguro en el banco.";
            return result;
        }

        result.status = InteractionStatus::SUCCESS;
        result.msg = "Retiro exitoso: Se ha transferido el artículo a tu inventario.";
        return result;
    }
};
