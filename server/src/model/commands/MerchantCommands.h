#ifndef MERCHANT_COMMANDS_H_
#define MERCHANT_COMMANDS_H_

#include <string>
#include <vector>

#include "../entities/Player.h"

#include "NPCCommand.h"

class BuyCommand: public NPCCommand {
public:
    bool execute(Player& player, const std::vector<std::string>& params) override {
        if (params.size() < 2)
            return false;

        try {
            uint32_t item_id = std::stoul(params[0]);
            uint16_t amount = static_cast<uint16_t>(std::stoul(params[1]));

            // CAMBIAR
            uint32_t total_price = 100 * amount;  // Precio simulado temporal

            return player.buy_item(item_id, amount, total_price);

        } catch (const std::exception&) {
            return false;
        }
    }
};

class SellCommand: public NPCCommand {
public:
    bool execute(Player& player, const std::vector<std::string>& params) override {
        if (params.size() < 2)
            return false;
        try {
            uint8_t slot_index = static_cast<uint8_t>(std::stoul(params[0]));
            uint16_t amount = static_cast<uint16_t>(std::stoul(params[1]));

            // CAMBIAR
            uint32_t unit_price = 50;  // Precio de reventa simulado
            return player.sell_item(slot_index, amount, unit_price);
        } catch (const std::exception&) {
            return false;
        }
    }
};

#endif  // MERCHANT_COMMANDS_H_
