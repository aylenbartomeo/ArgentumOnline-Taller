#pragma once

#include "NpcCommandHandler.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"
#include <unordered_map>
#include <string>

class TradeHandler : public NpcCommandHandler {
private:
    const ItemRegistry& registry;
    // Mapa de stock real del NPC: itemId -> cantidad disponible
    std::unordered_map<uint32_t, int>& npcStock;
    bool allowsSell;  // true: Merchant (compra/vende), false: Priest (solo vende)

public:
    TradeHandler(const ItemRegistry& registry, std::unordered_map<uint32_t, int>& stock,
                 bool allowsSell) :
            registry(registry), npcStock(stock), allowsSell(allowsSell) {}

    InteractionResult execute(Player& player, const NpcCommandDTO& dto) override {
        InteractionResult result;

        if (player.isDead()) {
            result.status = InteractionStatus::FAILURE;
            result.msg = "Los fantasmas no pueden comerciar.";
            return result;
        }

        uint32_t itemId = 0;
        try { 
            itemId = std::stoul(dto.arg); 
        } catch (...) { 
            return result; 
        }

        const Item* itemDef = registry.get_item(itemId);
        if (!itemDef) return result;

        uint32_t unitPrice = itemDef->getPrice(); 
        if (dto.type == NpcCommandType::BUY) {
            auto it = npcStock.find(itemId);
            if (it == npcStock.end() || it->second <= 0) {
                result.status = InteractionStatus::FAILURE;
                result.msg = "El comerciante no tiene stock de este artículo.";
                return result;
            }

            if (!player.removeGold(unitPrice)) {
                result.status = InteractionStatus::FAILURE;
                result.msg = "No tienes suficiente oro.";
                return result;
            }

            if (!player.addItem(itemId, 1)) {
                player.addGold(unitPrice); // Rollback
                result.status = InteractionStatus::FAILURE;
                result.msg = "Tu inventario está lleno.";
                return result;
            }

            npcStock[itemId]--;
            result.status = InteractionStatus::SUCCESS;
            result.msg = "Compra exitosa: +1 " + itemDef->getName();
            return result;
        }
        if (dto.type == NpcCommandType::SELL) {
            if (!allowsSell) {
                result.status = InteractionStatus::UNHANDLED;
                result.msg = "Los sacerdotes no pueden comprarte nada.";
                return result;
            }

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
                result.msg = "No se encuentra el item en el inventario.";
                return result;
            }

            // Quitamos el ítem al jugador y le pagamos la mitad del precio oficial
            player.removeItem(static_cast<uint8_t>(playerSlot), 1);
            player.addGold(unitPrice / 2);

            // El comerciante absorbe el ítem incrementando su stock
            npcStock[itemId]++;
            
            // Corregido: Cargamos el mensaje de éxito y devolvemos el result
            result.status = InteractionStatus::SUCCESS;
            result.msg = "Venta exitosa: compraste 1 " + itemDef->getName() + 
                                      " por " + std::to_string(unitPrice / 2) + " monedas de oro.";
            return result;
        }

        return result;
    }
};
