#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
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
        ss << "--- EXTRACTO BANCARIO ---\n";
        ss << "[ORO] " << goldInBank << " monedas.\n";

        std::vector<std::pair<const Item*, int>> potions;
        std::vector<std::pair<const Item*, int>> weapons;
        std::vector<std::pair<const Item*, int>> armors;
        std::vector<std::pair<const Item*, int>> others;

        bool hasItems = false;
        for (const auto& slot: slots) {
            if (!slot.is_empty()) {
                const Item* itemDef = registry.get_item(slot.item_id);
                if (itemDef) {
                    hasItems = true;
                    if (registry.get_consumable(slot.item_id)) {
                        potions.emplace_back(itemDef, slot.amount);
                    } else if (registry.get_weapon(slot.item_id)) {
                        weapons.emplace_back(itemDef, slot.amount);
                    } else if (registry.get_armor(slot.item_id)) {
                        armors.emplace_back(itemDef, slot.amount);
                    } else {
                        others.emplace_back(itemDef, slot.amount);
                    }
                }
            }
        }

        if (!hasItems) {
            ss << "(Tu bóveda no contiene ningún artículo en este momento)\n";
        } else {
            auto sortItems = [](std::vector<std::pair<const Item*, int>>& items) {
                std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
                    if (a.first->getPrice() != b.first->getPrice()) {
                        return a.first->getPrice() < b.first->getPrice();
                    }
                    return a.first->getName() < b.first->getName();
                });
            };

            sortItems(potions);
            sortItems(weapons);
            sortItems(armors);
            sortItems(others);

            auto formatItems = [&](const std::string& title,
                                   const std::vector<std::pair<const Item*, int>>& items) {
                if (items.empty())
                    return;
                ss << "\n[" << title << "]\n\n";
                for (const auto& [itemDef, quantity]: items) {
                    std::string name = itemDef->getName();
                    int padLength = 25 - static_cast<int>(name.length());
                    if (padLength < 3)
                        padLength = 3;
                    std::string paddedName = name + " " + std::string(padLength, '.');
                    ss << "- " << paddedName << " [Cant: " << quantity << "]\n";
                }
            };

            formatItems("POCIONES", potions);
            formatItems("ARMAS", weapons);
            formatItems("ARMADURAS", armors);
            formatItems("OTROS", others);
        }

        result.status = InteractionStatus::SUCCESS;
        result.msg = ss.str();
        return result;
    }
};
