#include "Priest.h"

#include <iostream>

#include "../World.h"
#include "../handlers/HealHandler.h"
#include "../handlers/ListStockHandler.h"
#include "../handlers/ResurrectHandler.h"
#include "../handlers/TradeHandler.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

Priest::Priest(uint32_t id, Position pos, const ItemRegistry& registry, 
        std::unordered_map<uint32_t, int> initialStock): id(id), pos(pos), 
        stock(std::move(initialStock)) {
    commandHandlers[NpcCommandType::RESPAWN] = std::make_unique<ResurrectHandler>();
    commandHandlers[NpcCommandType::HEAL] = std::make_unique<HealHandler>();

    // Inyección: Pasa su stock, pero NO permite que le vendan nada (allowsSell = false)
    commandHandlers[NpcCommandType::BUY] = std::make_unique<TradeHandler>(registry, stock, false);
    // En el constructor Priest::Priest(...)
    commandHandlers[NpcCommandType::LIST] =
            std::make_unique<ListStockHandler>(registry, stock, false);
}

InteractionResult Priest::beInteractedBy(Player& player) {
    InteractionResult result;
    if (player.isDead()) {
        result.msg = "[PRIEST] Tu alma vaga en el limbo. Usa /resucitar para volver a la vida.";
    } else {
        player.onActionStarted();
        result.msg = " [PRIEST] Saludos, viajero. ¿En qué puedo ayudarte hoy?";
    }
    return result;
}

InteractionResult Priest::handleCommand(Player& player, const NpcCommandDTO& dto) {
    auto it = commandHandlers.find(dto.type);
    if (it != commandHandlers.end()) {
        return it->second->execute(player, dto);
    }
    InteractionResult fallbackResult;
    fallbackResult.status = InteractionStatus::UNHANDLED;
    return fallbackResult;
}
