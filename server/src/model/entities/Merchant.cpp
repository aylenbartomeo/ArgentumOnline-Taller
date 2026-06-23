#include "Merchant.h"

#include <algorithm>
#include <sstream>
#include <utility>

#include "../handlers/ListStockHandler.h"
#include "../handlers/TradeHandler.h"
#include "model/entities/Player.h"

Merchant::Merchant(uint32_t id, Position pos, const ItemRegistry& registry,
                   std::unordered_map<uint32_t, int> initialStock):
        id(id), pos(pos), stock(std::move(initialStock)) {

    auto merchantFilter = [](const Item* item) {
        return !item->isMagic();  // Si es Weapon MAGIC, retorna false
    };
    // Inyección: El mercader compra y vende (allowsSell = true)
    commandHandlers[NpcCommandType::BUY] =
            std::make_unique<TradeHandler>(registry, stock, true, merchantFilter);
    commandHandlers[NpcCommandType::SELL] =
            std::make_unique<TradeHandler>(registry, stock, true, merchantFilter);
    commandHandlers[NpcCommandType::LIST] =
            std::make_unique<ListStockHandler>(registry, stock, true, merchantFilter);
}

InteractionResult Merchant::beInteractedBy(Player& player) {
    InteractionResult result;
    if (player.isDead()) {
        result.msg = "[MERCHANT] Si estas muerto, no me podes comprar nada joven viajero. Pedile "
                     "ayuda al sacerdote";
    } else {
        player.onActionStarted();
        result.msg = "[MERCHANT] ¡Bienvenido a mi humilde tienda! Si tienes oro, tengo maravillas "
                     "para ofrecerte. ¿Qué buscas hoy?";
    }
    result.status = InteractionStatus::SUCCESS;
    return result;
}

InteractionResult Merchant::handleCommand(Player& player, const NpcCommandDTO& dto) {
    auto it = commandHandlers.find(dto.type);
    if (it != commandHandlers.end()) {
        return it->second->execute(player, dto);
    }
    InteractionResult fallbackResult;
    fallbackResult.status = InteractionStatus::UNHANDLED;
    return fallbackResult;
}
