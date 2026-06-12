#pragma once

#include <sstream>
#include <iomanip>
#include "NpcCommandHandler.h"
#include "../items/ItemRegistry.h"

class ListStockHandler : public NpcCommandHandler {
private:
    const ItemRegistry& registry;
    const std::unordered_map<uint32_t, int>& npcStock;
    bool allowsSell;
    std::function<bool(const Item*)> isItemPermitted;

public:
    ListStockHandler(
        const ItemRegistry& registry, 
        const std::unordered_map<uint32_t, int>& stock,
        bool allowsSell,
        std::function<bool(const Item*)> filter = [](const Item*) { return true; })
        : registry(registry), npcStock(stock), allowsSell(allowsSell), isItemPermitted(filter) {}

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

        for (const auto& [itemId, quantity] : npcStock) {
            if (quantity <= 0) continue; // Saltamos si no hay stock

            const Item* itemDef = registry.get_item(itemId);
            if (!itemDef || !isItemPermitted(itemDef)) continue;

            uint32_t buyPrice = itemDef->getPrice();
            
            ss << "- " << itemDef->getName() 
               << " [ID: " << itemId << "]"
               << " | Cantidad: " << quantity 
               << " | Compra: " << buyPrice << "g";
            
            if (allowsSell) {
                ss << " | Venta: " << (buyPrice / 2) << "g";
            }
            ss << "\n";
        }

        result.status = InteractionStatus::SUCCESS;
        result.msg = ss.str();
        return result;
    }
};
