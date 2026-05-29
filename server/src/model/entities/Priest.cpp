#include "Priest.h"

#include <iostream>

#include "../World.h"
#include "../handlers/HealHandler.h"
#include "../handlers/ResurrectHandler.h"
#include "../handlers/TradeHandler.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

Priest::Priest(Position pos, const ItemRegistry& registry): pos(pos), stock() {
    commandHandlers[NpcCommandType::RESPAWN] = std::make_unique<ResurrectHandler>();
    commandHandlers[NpcCommandType::HEAL] = std::make_unique<HealHandler>();

    // El Sacerdote abre su tienda con stock inicial limitado
    stock[1001u] = 10;  // 10 Armaduras de cuero disponibles
    stock[1002u] = 5;   // 5 Armaduras de placas disponibles

    // Inyección: Pasa su stock, pero NO permite que le vendan nada (allowsSell = false)
    commandHandlers[NpcCommandType::BUY] = std::make_unique<TradeHandler>(registry, stock, false);
}

void Priest::beInteractedBy(Player& player) {
    std::cout << "[PRIEST] Interacción mística con: " << player.getName() << std::endl;
}

void Priest::handleCommand(Player& player, const NpcCommandDTO& dto) {
    auto it = commandHandlers.find(dto.type);
    if (it != commandHandlers.end()) {
        it->second->execute(player, dto);
    }
}
