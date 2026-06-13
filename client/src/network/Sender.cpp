#include "Sender.h"

#include <variant>

#include "common/include/dto/StartMoveDTO.h"

Sender::Sender(Protocol& protocol, Queue<CommandVariant>& commandQueue):
        protocol(protocol), commandQueue(commandQueue) {}

void Sender::run() {
    try {
        while (should_keep_running()) {
            CommandVariant cmd = commandQueue.pop();

            if (std::holds_alternative<StartMoveDTO>(cmd)) {
                protocol.sendStartMove(std::get<StartMoveDTO>(cmd));
            } else if (std::holds_alternative<StopMoveDTO>(cmd)) {
                protocol.sendStopMove();
            } else if (std::holds_alternative<AttackDTO>(cmd)) {
                protocol.sendAttack(std::get<AttackDTO>(cmd).targetId);
            } else if (std::holds_alternative<DropItemDTO>(cmd)) {
                protocol.sendDropItem(std::get<DropItemDTO>(cmd));
            } else if (std::holds_alternative<EquipItemDTO>(cmd)) {
                protocol.sendEquipItem(std::get<EquipItemDTO>(cmd));
            } else if (std::holds_alternative<UseItemDTO>(cmd)) {
                protocol.sendUseItem(std::get<UseItemDTO>(cmd));
            } else if (std::holds_alternative<GrabItemDTO>(cmd)) {
                protocol.sendGrabItem();
            } else if (std::holds_alternative<ChatDTO>(cmd)) {
                protocol.sendChat(std::get<ChatDTO>(cmd));
            } else if (std::holds_alternative<PrivateChatDTO>(cmd)) {
                protocol.sendPrivateChat(std::get<PrivateChatDTO>(cmd));
            } else if (std::holds_alternative<MeditateDTO>(cmd)) {
                protocol.sendMeditate();
            } else if (std::holds_alternative<NpcCommandDTO>(cmd)) {
                protocol.sendNpcCommand(std::get<NpcCommandDTO>(cmd));
            } else if (std::holds_alternative<ClanCommandDTO>(cmd)) {
                protocol.sendClanCommand(std::get<ClanCommandDTO>(cmd));
            } else if (std::holds_alternative<ResurrectDTO>(cmd)) {
                protocol.sendResurrect();
            } else if (std::holds_alternative<CheatDTO>(cmd)) {
                protocol.sendCheat(std::get<CheatDTO>(cmd));
            } else if (std::holds_alternative<ShootDTO>(cmd)) {
                protocol.sendShoot(std::get<ShootDTO>(cmd));
            } else if (std::holds_alternative<CreateCharacterDTO>(cmd)) {
                protocol.sendCreateCharacter(std::get<CreateCharacterDTO>(cmd));
            }
        }
    } catch (...) {}
}
