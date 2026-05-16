#include "Receiver.h"

Receiver::Receiver(Socket& skt, uint32_t clientId, Queue<GameEvent>& gameQueue):
        skt(skt), clientId(clientId), gameQueue(gameQueue), protocolo(skt) {}

bool Receiver::authenticatePlayer() {
    try {
        uint8_t action = this->protocolo.receiveAction();

        if (action == static_cast<uint8_t>(OPCODE::LOGIN)) {
            std::string clientUsername = this->protocolo.receiveUsername();
            this->protocolo.receivePassword();
            // std::string clientPassword = this->protocolo.receivePassword();

            JoinEvent joinEvent{this->clientId, clientUsername};
            this->gameQueue.push(joinEvent);

            return true;
        }
    } catch (...) {
        return false;
    }
    return false;
}

void Receiver::inGameCommunication() {
    try {
        while (should_keep_running()) {
            CommandDTO command;
            command.playerId = this->clientId;

            this->protocolo.receive_command(command);
            this->gameQueue.push(command);
        }
    } catch (const std::exception& e) {
        CommandDTO disconnectCommand;
        disconnectCommand.type = ActionType::DISCONNECT;
        disconnectCommand.playerId = this->clientId;
        this->gameQueue.push(disconnectCommand);
    }
}

void Receiver::run() {
    if (authenticatePlayer()) {
        inGameCommunication();
    }
}
