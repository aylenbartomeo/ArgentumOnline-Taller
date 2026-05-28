#include "Banker.h"
#include "GlobalBank.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"
#include "../World.h"
#include <iostream>
#include <string>
#include <sstream>

Banker::Banker(Position pos, GlobalBank& bank, const ItemRegistry& registry)
    : pos(pos), bank(bank), registry(registry) {}

void Banker::beInteractedBy(Player& player) {
    if (player.isDead()) {
        // pending: notify client via WorldEvent that player must be alive to use bank
        return;
    }
    // pending: notify client via WorldEvent to open bank UI
}

void Banker::handleCommand(Player& player, const NpcCommandDTO& dto) {
    if (player.isDead()) return; // pending: notify client that dead players cannot use bank

    uint32_t playerId = player.getDbId();

    // =========================================================================
    // /DEPOSITAR ORO <CANTIDAD>  o  /DEPOSITAR <ITEM_ID>
    // =========================================================================
    if (dto.type == NpcCommandType::DEPOSIT) {
        std::stringstream ss(dto.arg);
        std::string subComando;
        ss >> subComando;

        if (subComando == "oro") {
            uint32_t amount = 0;
            if (!(ss >> amount) || amount == 0 || player.getGold() < amount) return;

            player.removeGold(amount);
            bank.depositGold(playerId, amount);
            // pending: notify client deposit success
            return;
        }

        // Caso Ítem: dto.arg es el ID del ítem (ej: "4001")
        uint32_t itemId = 0;
        try { itemId = std::stoul(dto.arg); } catch (...) { return; }

        // Buscamos en qué slot del inventario del player está el ítem
        int playerSlot = -1;
        for (uint8_t i = 0; i < player.getSize(); ++i) {
            auto slotOpt = player.inspectSlot(i);
            if (slotOpt.has_value() && slotOpt->item_id == itemId) {
                playerSlot = i;
                break;
            }
        }

        if (playerSlot == -1) {
            // pending: notify client player doesn't have that item
            return;
        }

        // Intentamos meterlo al banco (1 unidad)
        if (!bank.depositItem(playerId, itemId, 1)) {
            // pending: notify client bank vault full
            return;
        }

        // Si el banco lo aceptó, lo borramos de la mochila del jugador
        player.removeItem(static_cast<uint8_t>(playerSlot), 1);
        
        const Item* itemDef = registry.get_item(itemId);
        std::string name = itemDef ? itemDef->getName() : "Objeto";
        // pending: notify client deposit item success
        return;
    }

    // =========================================================================
    // /RETIRAR ORO <CANTIDAD>  o  /RETIRAR <ITEM_ID>
    // =========================================================================
    if (dto.type == NpcCommandType::WITHDRAW) {
        std::stringstream ss(dto.arg);
        std::string subComando;
        ss >> subComando;

        if (subComando == "oro") {
            uint32_t amount = 0;
            if (!(ss >> amount) || amount == 0) return;

            if (!bank.withdrawGold(playerId, amount)) {
                // pending: notify client insufficient bank funds
                return;
            }

            player.addGold(amount);
            // pending: notify client withdraw success
            return;
        }

        // Caso Ítem: dto.arg es el ID a retirar
        uint32_t itemId = 0;
        try { itemId = std::stoul(dto.arg); } catch (...) { return; }

        // Intentamos sacar 1 unidad del banco
        uint16_t withdrawnAmount = bank.withdrawItemById(playerId, itemId, 1);
        if (withdrawnAmount == 0) {
            // pending: notify client item not in vault
            return;
        }

        // Intentamos meterlo en la mochila del player
        if (!player.addItem(itemId, 1)) {
            // Rollback atómico: si no entra en la mochila, vuelve directo a la bóveda
            bank.depositItem(playerId, itemId, 1);
            // pending: notify client inventory full
            return;
        }

        const Item* itemDef = registry.get_item(itemId);
        std::string name = itemDef ? itemDef->getName() : "Objeto";
        // pending: notify client withdraw item success
        return;
    }
}