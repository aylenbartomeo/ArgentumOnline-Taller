#include "Merchant.h"

#include "../handlers/TradeHandler.h"
#include "model/entities/Player.h"

Merchant::Merchant(Position pos, const ItemRegistry& registry): pos(pos), stock() {
    // El comerciante abre con sus artículos locales
    stock[4001u] = 5;  // 5 Espadas
    stock[1001u] = 3;  // 3 Armaduras

    // Inyección: El mercader compra y vende (allowsSell = true)
    commandHandlers[NpcCommandType::BUY] = std::make_unique<TradeHandler>(registry, stock, true);
    commandHandlers[NpcCommandType::SELL] = std::make_unique<TradeHandler>(registry, stock, true);
}

void Merchant::beInteractedBy(Player& player) {
    if (player.isDead())
        return;
    // TODO: Abrir interfaz de la grilla de la tienda
}

void Merchant::handleCommand(Player& player, const NpcCommandDTO& dto) {
    auto it = commandHandlers.find(dto.type);
    if (it != commandHandlers.end()) {
        it->second->execute(player, dto);
    }
}
