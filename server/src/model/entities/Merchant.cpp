#include "Merchant.h"

#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"
#include "../World.h"
#include <iostream>
#include <string>
#include <algorithm>

Merchant::Merchant(Position pos, const ItemRegistry& registry) 
    : position(pos), registry(registry) {}

void Merchant::beInteractedBy(Player& player) {
    std::cout << "[MERCHANT] Hola " << player.getName() << ", ¿qué deseas comprar o vender?" << std::endl;

    // pending: push WorldEvent to outgoingEvents to open merchant shop UI in client
}

void Merchant::handleCommand(Player& player, const NpcCommandDTO& dto) {
    // 1. Filtrar comandos ajenos al rubro comercial
    if (dto.type != NpcCommandType::BUY && dto.type != NpcCommandType::SELL) {
        // pending: notify client that command is unsupported for merchant
        return;
        return;
    }

    int itemId = 0;

    // 2. Parsear únicamente el arg1 (ID del ítem). Ignoramos arg2.
    try {
        itemId = std::stoi(dto.arg);
    } catch (const std::exception& e) {
        // pending: notify client that item code is invalid
        return;
    }

    // 3. Validar existencia del ítem mediante tu get_item real
    const Item* itemDef = registry.get_item(itemId);
    if (!itemDef) {
        // pending: notify client that item does not exist
        return;
    }

    // 4. Filtro de Hechizos (Regla del enunciado)
    // HAY QUE REFACTORIZAR PARA QUE REGISTRY DISTINGA LOS MAGICOS
    if (itemDef->getName().find("Pergamino") != std::string::npos || 
        itemDef->getName().find("Hechizo") != std::string::npos ||
        itemDef->getName().find("Baculo") != std::string::npos ||
        itemDef->getName().find("Vara") != std::string::npos) {
        
        // pending: notify client that merchant doesn't trade magical items
        return;
    }

    // Fijamos el precio base y la cantidad unitaria (siempre 1)
    int unitPrice = 100; // Mockeado por ahora --> item.getPrice()

    // =========================================================================
    // COMPRA DEL PLAYER
    // =========================================================================
    if (dto.type == NpcCommandType::BUY) {
        // Validar si el jugador tiene el oro para comprar 1 unidad
        if (!player.removeGold(unitPrice)) {
            // pending: notify client insufficient gold
            return;
        }
        // Validar si entra 1 unidad en la mochila
        if (!player.addItem(static_cast<uint32_t>(itemId), 1)) {
            player.addGold(unitPrice);
            // pending: notify client inventory full
            return;
        }

        // pending: notify client purchase success

    // =========================================================================
    // VENTA DEL PLAYER
    // =========================================================================
    } else if (dto.type == NpcCommandType::SELL) {
        int targetSlot = -1;

        // Buscamos el ítem celda por celda inspeccionando los slots reales del inventario
        for (uint8_t i = 0; i < player.getSize(); ++i) {
            auto slotOpt = player.inspectSlot(i);
            if (slotOpt.has_value() && slotOpt->item_id == static_cast<uint32_t>(itemId)) {
                targetSlot = i;
                break; 
            }
        }

        if (targetSlot == -1) {
            // pending: notify client player doesn't have item
            return;
        }

        // El mercader compra al 50% del valor comercial
        int goldToGive = unitPrice / 2;

        // Removemos 1 unidad de ese slot específico y le pagamos al jugador
        player.removeItem(static_cast<uint8_t>(targetSlot), 1);
            player.addGold(static_cast<uint32_t>(goldToGive));

        // Registramos en el stock interno con casteo seguro
        stock[static_cast<uint32_t>(itemId)] += 1;

        // pending: notify client sell success
    }
}
