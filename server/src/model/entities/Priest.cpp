#include "Priest.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"
#include "../World.h"
#include <iostream>
#include <string>
#include <algorithm>

Priest::Priest(Position pos, const ItemRegistry& registry) 
    : pos(pos), registry(registry) {}

void Priest::beInteractedBy(Player& player) {
    std::cout << "[PRIEST] Interacción mística con el jugador: " << player.getName() << std::endl;

    // Si el jugador está muerto, se indicaría al cliente que muestre la opción de resurrección.
    if (player.isDead()) {
        // pending: push WorldEvent to outgoingEvents to prompt '/resucitar' in client UI
        return;
    }

    // Si está vivo, se indicaría al cliente que abra la interfaz del Sacerdote.
    // pending: push WorldEvent to outgoingEvents to open priest shop UI
}

void Priest::handleCommand(Player& player, const NpcCommandDTO& dto) {
    if (dto.type == NpcCommandType::RESPAWN) {
        if (!player.isDead()) {
            // pending: notify client that player is already alive
            return;
        }

        // Ejecuta la resurrección real en tu Player (le quita el estado fantasma, resetea HP)
        player.resurrect(); 

        // pending: notify client that resurrection succeeded
        return;
    }
    if (dto.type == NpcCommandType::HEAL) {
        if (player.isDead()) {
            // pending: notify client that dead players must resurrect first
            return;
        }

        // Si ya está lleno de vida y maná, no gasta energía divina
        if (player.getHp() == player.getMaxHp() && player.getMana() == player.getMaxMana()) {
            // pending: notify client that HP/Mana are already at max
            return;
        }

        // Restauramos ambas estadísticas al tope mediante la API de tu Player
        player.restoreHp();
        player.restoreMana();

        // pending: notify client that HP and mana restored
        return;
    }

    // Si está muerto, no puede comerciar
    if (player.isDead()) {
        // pending: notify client that ghosts cannot trade
        return;
    }

    int itemId = 0;
    try {
        itemId = std::stoi(dto.arg);
    } catch (const std::exception& e) {
        // pending: notify client that item code is invalid
        return;
    }

    const Item* itemDef = registry.get_item(itemId);
    if (!itemDef) {
        // pending: notify client that item does not exist
        return;
    }

    int unitPrice = 100; // Precio místico acorde a tests

    // --- FLUJO DE COMPRA (BUY) ---
    if (dto.type == NpcCommandType::BUY) {
        if (!player.removeGold(static_cast<uint32_t>(unitPrice))) {
            // pending: notify client insufficient gold
            return;
        }

        if (!player.addItem(static_cast<uint32_t>(itemId), 1)) {
            player.addGold(static_cast<uint32_t>(unitPrice)); // Rollback atómico
            // pending: notify client inventory full
            return;
        }

        // pending: notify client purchase success

    // --- FLUJO DE VENTA (SELL) ---
    } else if (dto.type == NpcCommandType::SELL) {
        int targetSlot = -1;

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

        int goldToGive = unitPrice / 2;

        player.removeItem(static_cast<uint8_t>(targetSlot), 1);
        player.addGold(static_cast<uint32_t>(goldToGive));
        // pending: notify client sell success
    }

}
