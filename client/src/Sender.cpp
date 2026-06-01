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
                protocol.send_start_move(std::get<StartMoveDTO>(cmd));
            } else if (std::holds_alternative<StopMoveDTO>(cmd)) {
                protocol.send_stop_move();
            } else if (std::holds_alternative<AttackDTO>(cmd)) {
                protocol.send_attack();
            } else if (std::holds_alternative<DropItemDTO>(cmd)) {
                protocol.send_drop_item(std::get<DropItemDTO>(cmd));
            } else if (std::holds_alternative<EquipItemDTO>(cmd)) {
                protocol.send_equip_item(std::get<EquipItemDTO>(cmd));
            } else if (std::holds_alternative<UseItemDTO>(cmd)) {
                protocol.send_use_item(std::get<UseItemDTO>(cmd));
            } else if (std::holds_alternative<GrabItemDTO>(cmd)) {
                protocol.send_grab_item();
            } else if (std::holds_alternative<ChatDTO>(cmd)) {
                protocol.send_chat(std::get<ChatDTO>(cmd));
            } else if (std::holds_alternative<PrivateChatDTO>(cmd)) {
                protocol.send_private_chat(std::get<PrivateChatDTO>(cmd));
            }
        }
    } catch (...) {}
}
