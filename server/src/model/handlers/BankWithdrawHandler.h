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

    bool execute(Player& player, const NpcCommandDTO& dto) override {
        if (dto.type != NpcCommandType::WITHDRAW)
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
            if (!(ss >> amount) || amount == 0)
                return true;

            if (!bankInstance.withdrawGold(playerId, amount))
                return true;

            player.addGold(amount);
            return true;
        }

        // ---- Subcomando: Ítem ----
        uint32_t itemId = 0;
        try {
            itemId = std::stoul(dto.arg);
        } catch (...) {
            return true;
        }

        uint16_t withdrawnAmount = bankInstance.withdrawItemById(playerId, itemId, 1);
        if (withdrawnAmount == 0)
            return true;

        if (!player.addItem(itemId, 1)) {
            bankInstance.depositItem(playerId, itemId, 1);  // Rollback atómico
        }
        return true;
    }
};
