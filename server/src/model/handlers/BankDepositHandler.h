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

    bool execute(Player& player, const NpcCommandDTO& dto) override {
        if (dto.type != NpcCommandType::DEPOSIT)
            return false;
        if (player.isDead())
            return true;

        uint32_t playerId = player.getDbId();
        std::stringstream ss(dto.arg);
        std::string subComando;
        ss >> subComando;

        // ---- Subcomando: Oro ----
        if (subComando == "oro") {
            uint32_t amount = 0;
            if (!(ss >> amount) || amount == 0 || player.getGold() < amount)
                return true;

            player.removeGold(amount);
            bankInstance.depositGold(playerId, amount);
            // TODO: Notificar éxito por red
            return true;
        }

        // ---- Subcomando: Ítem (dto.arg es el ID del ítem) ----
        uint32_t itemId = 0;
        try {
            itemId = std::stoul(dto.arg);
        } catch (...) {
            return true;
        }

        int playerSlot = -1;
        for (uint8_t i = 0; i < player.getSize(); ++i) {
            auto slotOpt = player.inspectSlot(i);
            if (slotOpt.has_value() && slotOpt->item_id == itemId) {
                playerSlot = i;
                break;
            }
        }

        if (playerSlot == -1)
            return true;

        if (!bankInstance.depositItem(playerId, itemId, 1)) {
            // TODO: Notificar banco lleno
            return true;
        }

        player.removeItem(static_cast<uint8_t>(playerSlot), 1);
        return true;
    }
};
