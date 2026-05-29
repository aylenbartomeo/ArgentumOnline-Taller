#pragma once

#include <string>
#include <unordered_map>

#include "../entities/Player.h"
#include "../items/ItemRegistry.h"

#include "NpcCommandHandler.h"

class TradeHandler: public NpcCommandHandler {
private:
    const ItemRegistry& registry;
    // Mapa de stock real del NPC: itemId -> cantidad disponible
    std::unordered_map<uint32_t, int>& npcStock;
    bool allowsSell;  // true: Merchant (compra/vende), false: Priest (solo vende)

public:
    TradeHandler(const ItemRegistry& registry, std::unordered_map<uint32_t, int>& stock,
                 bool allowsSell):
            registry(registry), npcStock(stock), allowsSell(allowsSell) {}

    bool execute(Player& player, const NpcCommandDTO& dto) override {
        if (player.isDead())
            return true;

        uint32_t itemId = 0;
        try {
            itemId = std::stoul(dto.arg);
        } catch (...) {
            return true;
        }

        // Validamos que el ítem exista en la base de datos del juego
        const Item* itemDef = registry.get_item(itemId);
        if (!itemDef)
            return true;

        int unitPrice = 100;  // Precio HARCODEADO --> Implementar itemDef.getPrice();

        if (dto.type == NpcCommandType::BUY) {
            auto it = npcStock.find(itemId);

            // Si el NPC no tiene el ítem en su mapa o su stock llegó a 0, rebota
            if (it == npcStock.end() || it->second <= 0) {
                // TODO: Notificar por red "No hay stock disponible"
                return true;
            }

            // Flujo económico del jugador
            if (!player.removeGold(static_cast<uint32_t>(unitPrice)))
                return true;

            if (!player.addItem(itemId, 1)) {
                player.addGold(static_cast<uint32_t>(unitPrice));  // Rollback oro
                return true;
            }

            // Descontamos del stock del NPC de forma directa
            npcStock[itemId]--;
            return true;
        }

        if (dto.type == NpcCommandType::SELL) {
            if (!allowsSell)
                return true;  // El Priest ignora este comando por completo

            int playerSlot = -1;
            for (uint8_t i = 0; i < player.getSize(); ++i) {
                auto slotOpt = player.inspectSlot(i);
                if (slotOpt.has_value() && slotOpt->item_id == itemId) {
                    playerSlot = i;
                    break;
                }
            }
            if (playerSlot == -1)
                return true;  // No lo tiene

            // Quitamos el ítem al jugador y le pagamos la mitad
            player.removeItem(static_cast<uint8_t>(playerSlot), 1);
            player.addGold(static_cast<uint32_t>(unitPrice / 2));

            // El comerciante compra el ítem: incrementa su stock local.
            // Si el ítem no figuraba en su mapa original, se inicializa en 1 de forma automática.
            npcStock[itemId]++;
            return true;
        }

        return false;
    }
};
