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
            } else if (std::holds_alternative<GrabItemDTO>(cmd)) {
                protocol.send_grab_item();
            }
        }
    } catch (...) {}
}
