#pragma once

#include <sstream>
#include <vector>

#include "../entities/GlobalBank.h"
#include "../entities/Player.h"
#include "../items/ItemRegistry.h"

#include "NpcCommandHandler.h"

class BankListHandler: public NpcCommandHandler {
private:
    GlobalBank& bankInstance;
    const ItemRegistry& registry;

public:
    BankListHandler(GlobalBank& bankInstance, const ItemRegistry& registry):
            bankInstance(bankInstance), registry(registry) {}

    InteractionResult execute(Player& player, const NpcCommandDTO& dto) override {
        InteractionResult result;

        // 1. Validar si el comando corresponde a este handler
        if (dto.type != NpcCommandType::LIST) {
            result.status = InteractionStatus::UNHANDLED;
            return result;
        }

        // 2. Regla de negocio: Los fantasmas no pueden interactuar con el plano físico del banco
        if (player.isDead()) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "Los fantasmas no pueden inspeccionar cuentas bancarias.";
            return result;
        }

        uint32_t playerId = player.getDbId();
        uint32_t goldInBank = bankInstance.getBankGold(playerId);
        const auto& slots = bankInstance.getBankSlots(playerId);

        std::stringstream ss;
        ss << "--- extracto de bóveda bancaria ---\n";
        ss << "Oro resguardado: " << goldInBank << " monedas.\n";
        ss << "Artículos depositados:\n";

        bool hasItems = false;
        for (const auto& slot: slots) {
            if (!slot.is_empty()) {
                const Item* itemDef = registry.get_item(slot.item_id);
                if (itemDef) {
                    ss << "- " << itemDef->getName() << " [ID: " << slot.item_id << "]"
                       << " x" << slot.amount << "\n";
                    hasItems = true;
                }
            }
        }

        if (!hasItems) {
            ss << "(Tu bóveda no contiene ningún artículo en este momento)\n";
        }

        result.status = InteractionStatus::SUCCESS;
        result.msg = ss.str();
        return result;
    }
};
