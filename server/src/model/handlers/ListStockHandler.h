#pragma once

#include <algorithm>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../items/ItemRegistry.h"

#include "NpcCommandHandler.h"

class ListStockHandler: public NpcCommandHandler {
private:
    const ItemRegistry& registry;
    const std::unordered_map<uint32_t, int>& npcStock;
    bool allowsSell;
    std::function<bool(const Item*)> isItemPermitted;

public:
    ListStockHandler(
            const ItemRegistry& registry, const std::unordered_map<uint32_t, int>& stock,
            bool allowsSell,
            std::function<bool(const Item*)> filter = [](const Item*) { return true; }):
            registry(registry), npcStock(stock), allowsSell(allowsSell), isItemPermitted(filter) {}

    InteractionResult execute(Player& player, const NpcCommandDTO& dto) override {
        InteractionResult result;

        if (dto.type != NpcCommandType::LIST) {
            result.status = InteractionStatus::UNHANDLED;
            return result;
        }

        if (player.isDead()) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "Tus ojos espectrales no pueden comprender el comercio de los vivos.";
            return result;
        }

        if (npcStock.empty()) {
            result.status = InteractionStatus::SUCCESS;
            result.msg = "El establecimiento se encuentra sin mercadería en este momento.";
            return result;
        }

        std::stringstream ss;
        ss << "--- CATÁLOGO DISPONIBLE ---\n";

        std::vector<std::pair<const Item*, int>> potions;
        std::vector<std::pair<const Item*, int>> weapons;
        std::vector<std::pair<const Item*, int>> armors;
        std::vector<std::pair<const Item*, int>> others;

        for (const auto& [itemId, quantity]: npcStock) {
            if (quantity <= 0)
                continue;

            const Item* itemDef = registry.get_item(itemId);
            if (!itemDef || !isItemPermitted(itemDef))
                continue;

            if (registry.get_consumable(itemId)) {
                potions.emplace_back(itemDef, quantity);
            } else if (registry.get_weapon(itemId)) {
                weapons.emplace_back(itemDef, quantity);
            } else if (registry.get_armor(itemId)) {
                armors.emplace_back(itemDef, quantity);
            } else {
                others.emplace_back(itemDef, quantity);
            }
        }

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
                uint32_t buyPrice = itemDef->getPrice();
                std::string name = itemDef->getName();

                int padLength = 25 - static_cast<int>(name.length());
                if (padLength < 3)
                    padLength = 3;
                std::string paddedName = name + " " + std::string(padLength, '.');

                ss << "- " << paddedName << " Compra: " << buyPrice << "g";
                if (allowsSell) {
                    ss << " | Venta: " << (buyPrice / 2) << "g";
                }
                ss << " | Disp: " << quantity << "\n";
            }
        };

        formatItems("POCIONES", potions);
        formatItems("ARMAS", weapons);
        formatItems("ARMADURAS", armors);
        formatItems("OTROS", others);

        result.status = InteractionStatus::SUCCESS;
        result.msg = ss.str();
        return result;
    }
};
