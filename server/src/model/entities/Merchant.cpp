#include "Merchant.h"

#include "../handlers/TradeHandler.h"
#include "model/entities/Player.h"

Merchant::Merchant(uint32_t id, Position pos, const ItemRegistry& registry):
        id(id), pos(pos), stock() {
    // El comerciante abre con sus artículos locales
    stock[2000u] = 5;  // 5 Espadas
    stock[1000u] = 3;  // 3 Armaduras de cuero
    
    auto merchantFilter = [](const Item* item) {
        return !item->isMagic(); // Si es Weapon MAGIC, retorna false
    };
    // Inyección: El mercader compra y vende (allowsSell = true)
    commandHandlers[NpcCommandType::BUY] = std::make_unique<TradeHandler>(registry, stock, true, merchantFilter);
    commandHandlers[NpcCommandType::SELL] = std::make_unique<TradeHandler>(registry, stock, true, merchantFilter);
}

InteractionResult Merchant::beInteractedBy(Player& player) {
    InteractionResult result;
    if (player.isDead()) {
        result.msg = "[MERCHANT] Si estas muerto, no me podes comprar nada joven viajero. Pedile "
                     "ayuda al sacerdote";
    } else {
        result.msg = "[MERCHANT] Saludos, viajero. ¿Lo puedo ayudar con algo?";
    }
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
