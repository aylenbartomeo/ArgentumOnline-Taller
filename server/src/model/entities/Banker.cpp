#include "Banker.h"

#include "../handlers/BankDepositHandler.h"
#include "../handlers/BankListHandler.h"
#include "../handlers/BankWithdrawHandler.h"

Banker::Banker(uint32_t id, Position pos, GlobalBank& bankInstance, const ItemRegistry& registry):
        id(id), pos(pos) {
    commandHandlers[NpcCommandType::DEPOSIT] =
            std::make_unique<BankDepositHandler>(bankInstance, registry);
    commandHandlers[NpcCommandType::WITHDRAW] = std::make_unique<BankWithdrawHandler>(bankInstance);
    commandHandlers[NpcCommandType::LIST] =
            std::make_unique<BankListHandler>(bankInstance, registry);
}

InteractionResult Banker::beInteractedBy(Player& player) {
    InteractionResult result;
    if (player.isDead()) {
        result.msg =
                "[BANKER] PELIGRO ESTAMOS EN EL CORRALITO. Mentira buen hombre, solo se "
                "encuentra muerto. Hable con San Pedro, el sacerdote del pueblo, para resucitar";
    } else {
        player.onActionStarted();
        result.msg = "[BANKER] Saludos, viajero. ¿Lo puedo ayudar con algo?";
    }
    return result;
}

InteractionResult Banker::handleCommand(Player& player, const NpcCommandDTO& dto) {
    auto it = commandHandlers.find(dto.type);
    if (it != commandHandlers.end()) {
        return it->second->execute(player, dto);
    }
    InteractionResult fallbackResult;
    fallbackResult.status = InteractionStatus::UNHANDLED;
    return fallbackResult;
}
