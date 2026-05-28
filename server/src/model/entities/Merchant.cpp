#include "Merchant.h"

#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"
#include "../World.h"
#include <iostream>
#include <string>
#include <algorithm>

Merchant::Merchant(Position pos, const ItemRegistry& registry) 
    : position(pos), registry(registry) {}

void Merchant::beInteractedBy(Player& player, std::vector<WorldEvent>& outgoingEvents) {
    std::cout << "[MERCHANT] Hola " << player.getName() << ", ¿qué deseas comprar o vender?" << std::endl;

    // Generamos el evento para el cliente. 
    // El mensaje puede ser un string plano o un formato que tu capa de red traduzca
    // para abrir la ventana de comercio ("OPEN_SHOP_MERCHANT").
    outgoingEvents.push_back({ player.getDbId(), "OPEN_SHOP" });
}

void Merchant::handleCommand(Player& player, const NpcCommandDTO& dto, std::vector<WorldEvent>& outgoingEvents) {
    // 1. Filtrar comandos ajenos al rubro comercial
    if (dto.type != NpcCommandType::BUY && dto.type != NpcCommandType::SELL) {
        WorldEvent errEvent;
        errEvent.targetDbId = static_cast<uint32_t>(player.getDbId());
        errEvent.message = "El comerciante dice: 'Yo no entiendo esa orden, solo compro y vendo artículos.'";
        outgoingEvents.push_back(errEvent);
        return;
    }

    int itemId = 0;

    // 2. Parsear únicamente el arg1 (ID del ítem). Ignoramos arg2.
    try {
        itemId = std::stoi(dto.arg);
    } catch (const std::exception& e) {
        WorldEvent errEvent;
        errEvent.targetDbId = static_cast<uint32_t>(player.getDbId());
        errEvent.message = "Código de artículo inválido.";
        outgoingEvents.push_back(errEvent);
        return;
    }

    // 3. Validar existencia del ítem mediante tu get_item real
    const Item* itemDef = registry.get_item(itemId);
    if (!itemDef) {
        WorldEvent errEvent;
        errEvent.targetDbId = static_cast<uint32_t>(player.getDbId());
        errEvent.message = "Ese artículo no existe en este mundo.";
        outgoingEvents.push_back(errEvent);
        return;
    }

    // 4. Filtro de Hechizos (Regla del enunciado)
    // HAY QUE REFACTORIZAR PARA QUE REGISTRY DISTINGA LOS MAGICOS
    if (itemDef->getName().find("Pergamino") != std::string::npos || 
        itemDef->getName().find("Hechizo") != std::string::npos ||
        itemDef->getName().find("Baculo") != std::string::npos ||
        itemDef->getName().find("Vara") != std::string::npos) {
        
        WorldEvent errEvent;
        errEvent.targetDbId = static_cast<uint32_t>(player.getDbId());
        errEvent.message = "El comerciante dice: 'Yo no comercio con magia. Ve a ver a un Sacerdote.'";
        outgoingEvents.push_back(errEvent);
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
            WorldEvent errEvent;
            errEvent.targetDbId = static_cast<uint32_t>(player.getDbId());
            errEvent.message = "No tienes suficiente oro para comprar este artículo.";
            outgoingEvents.push_back(errEvent);
            return;
        }
        // Validar si entra 1 unidad en la mochila
        if (!player.addItem(static_cast<uint32_t>(itemId), 1)) {
            player.addGold(unitPrice);
            WorldEvent errEvent;
            errEvent.targetDbId = static_cast<uint32_t>(player.getDbId());
            errEvent.message = "No tienes espacio suficiente en tu mochila.";
            outgoingEvents.push_back(errEvent);
            return;
        }

        WorldEvent successEvent;
        successEvent.targetDbId = static_cast<uint32_t>(player.getDbId());
        successEvent.message = "Compraste 1x " + itemDef->getName() + " por " + std::to_string(unitPrice) + " monedas de oro.";
        outgoingEvents.push_back(successEvent);

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
            WorldEvent errEvent;
            errEvent.targetDbId = static_cast<uint32_t>(player.getDbId());
            errEvent.message = "No tienes ese artículo en tu inventario.";
            outgoingEvents.push_back(errEvent);
            return;
        }

        // El mercader compra al 50% del valor comercial
        int goldToGive = unitPrice / 2;

        // Removemos 1 unidad de ese slot específico y le pagamos al jugador
        player.removeItem(static_cast<uint8_t>(targetSlot), 1);
        player.addGold(static_cast<uint32_t>(goldToGive));

        // Registramos en el stock interno con casteo seguro
        stock[static_cast<uint32_t>(itemId)] += 1;

        WorldEvent successEvent;
        successEvent.targetDbId = static_cast<uint32_t>(player.getDbId());
        successEvent.message = "Vendiste 1x " + itemDef->getName() + " por " + std::to_string(goldToGive) + " monedas de oro.";
        outgoingEvents.push_back(successEvent);
    }
}
