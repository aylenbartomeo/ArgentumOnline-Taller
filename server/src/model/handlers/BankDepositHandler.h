#pragma once

#include <sstream>
#include <string>

#include "../entities/GlobalBank.h"
#include "../entities/Player.h"
#include "../items/ItemRegistry.h"

#include "NpcCommandHandler.h"

class BankDepositHandler: public NpcCommandHandler {
private:
    GlobalBank& bankInstance;
    const ItemRegistry& registry;

public:
    BankDepositHandler(GlobalBank& bankInstance, const ItemRegistry& registry):
            bankInstance(bankInstance), registry(registry) {}

    InteractionResult execute(Player& player, const NpcCommandDTO& dto) override {
        InteractionResult result;

        // 1. Validar si el comando corresponde a este handler
        if (dto.type != NpcCommandType::DEPOSIT) {
            result.status = InteractionStatus::UNHANDLED;
            return result;
        }

        // 2. Regla de negocio: Los fantasmas no pueden interactuar con la materia física del banco
        if (player.isDead()) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "Los fantasmas no pueden depositar bienes.";
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
                result.msg = "Cantidad de oro inválida. Uso: /depositar oro <cantidad>";
                return result;
            }

            if (player.getGold() < amount) {
                result.status = InteractionStatus::FAILURE;
                result.msg =
                        "No tienes suficiente oro en tu inventario para depositar esa cantidad.";
                return result;
            }

            // Procesamos la transacción de forma segura
            player.removeGold(amount);
            bankInstance.depositGold(playerId, amount);

            result.status = InteractionStatus::SUCCESS;
            result.msg = "Depósito exitoso: Guardaste " + std::to_string(amount) +
                         " monedas de oro en tu cuenta.";
            return result;
        }

        // Usamos 'subComando' para asegurarnos de parsear solo el token numérico aislado
        uint32_t itemId = 0;
        try {
            itemId = std::stoul(subComando);
        } catch (...) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "Argumento inválido. Uso: /depositar oro <cant> o /depositar <itemId>";
            return result;
        }

        // Validamos contra el registro que el ítem realmente exista
        const Item* itemDef = registry.get_item(itemId);
        if (!itemDef) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "El artículo especificado no existe en el registro.";
            return result;
        }

        // Buscamos si el jugador efectivamente posee el ítem en su inventario
        int playerSlot = -1;
        for (uint8_t i = 0; i < player.getSize(); ++i) {
            auto slotOpt = player.inspectSlot(i);
            if (slotOpt.has_value() && slotOpt->item_id == itemId) {
                playerSlot = i;
                break;
            }
        }

        if (playerSlot == -1) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "No tienes ese artículo en tu inventario.";
            return result;
        }

        // Intentamos depositarlo en la bóveda del banco (aquí se usa el find_if que refactorizamos
        // antes)
        if (!bankInstance.depositItem(playerId, itemId, 1)) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "Tu bovéda bancaria está llena. No hay espacio para más artículos.";
            return result;
        }

        // Si el banco lo aceptó con éxito, lo removemos de la mochila del jugador
        player.removeItem(static_cast<uint8_t>(playerSlot), 1);
        result.status = InteractionStatus::SUCCESS;
        result.msg = "Depósito exitoso: Protegiste 1 " + itemDef->getName() + " en tu bovéda.";
        return result;
    }
};
