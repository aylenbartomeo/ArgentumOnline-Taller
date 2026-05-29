#include "Banker.h"

#include "../handlers/BankDepositHandler.h"
#include "../handlers/BankWithdrawHandler.h"

Banker::Banker(Position pos, GlobalBank& bankInstance, const ItemRegistry& registry): pos(pos) {
    commandHandlers[NpcCommandType::DEPOSIT] =
            std::make_unique<BankDepositHandler>(bankInstance, registry);
    commandHandlers[NpcCommandType::WITHDRAW] = std::make_unique<BankWithdrawHandler>(bankInstance);
}

void Banker::beInteractedBy(Player& player) {
    if (player.isDead())
        return;
    // TODO: Disparar orden de abrir UI de banco
}

void Banker::handleCommand(Player& player, const NpcCommandDTO& dto) {
    auto it = commandHandlers.find(dto.type);
    if (it != commandHandlers.end()) {
        it->second->execute(player, dto);
    }
}
