#ifndef BANKER_COMMANDS_H_
#define BANKER_COMMANDS_H_

#include <stdexcept>
#include <string>
#include <vector>

#include "../entities/Player.h"

#include "NPCCommand.h"

class DepositCommand: public NPCCommand {
public:
    bool execute(Player& /*player*/, const std::vector<std::string>& params) override {
        if (params.size() < 2)
            return false;

        const std::string& deposit_type = params[0];

        try {
            if (deposit_type == "gold") {
                uint32_t amount = static_cast<uint32_t>(std::stoul(params[1]));
                // return player.deposit_gold(amount);
                (void)amount;
                return false;  // TODO: Conectar con player.deposit_gold()
            } else if (deposit_type == "item") {
                if (params.size() < 3)
                    return false;

                uint8_t inv_slot = static_cast<uint8_t>(std::stoul(params[1]));
                uint16_t amount = static_cast<uint16_t>(std::stoul(params[2]));

                // return player.deposit_item(inv_slot, amount);
                (void)inv_slot;
                (void)amount;
                return false;  // TODO: Conectar con player.deposit_item()
            }
        } catch (const std::exception&) {
            return false;
        }

        return false;
    }
};

class WithdrawCommand: public NPCCommand {
public:
    bool execute(Player& /*player*/, const std::vector<std::string>& params) override {
        if (params.size() < 2)
            return false;

        const std::string& withdraw_type = params[0];

        try {
            if (withdraw_type == "gold") {
                uint32_t amount = static_cast<uint32_t>(std::stoul(params[1]));

                // NOTA: Acá el Banquero debería validar primero si el jugador
                // tiene esa cantidad de oro guardada en su cuenta bancaria persistente.
                // Si la tiene, se la descontamos del banco y le ordenamos al Player retirarla:
                // return player.withdraw_gold(amount);
                (void)amount;
                return false;  // TODO: Conectar con player.withdraw_gold()
            } else if (withdraw_type == "item") {
                if (params.size() < 3)
                    return false;

                uint32_t item_id = static_cast<uint32_t>(std::stoul(params[1]));
                uint16_t amount = static_cast<uint16_t>(std::stoul(params[2]));

                // NOTA: Acá el Banquero debería validar primero si el jugador
                // tiene esa cantidad de ítems guardada en su cuenta bancaria persistente.
                // return player.withdraw_item(item_id, amount);
                (void)item_id;
                (void)amount;
                return false;  // TODO: Conectar con player.withdraw_item()
            }
        } catch (const std::exception&) {
            return false;
        }

        return false;
    }
};

#endif  // BANKER_COMMANDS_H_
