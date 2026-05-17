#include "Receiver.h"

Receiver::Receiver(Socket& skt, uint32_t clientId, Queue<GameEvent>& gameQueue):
        skt(skt), clientId(clientId), gameQueue(gameQueue), protocolo(skt) {}

bool Receiver::authenticatePlayer() {
    try {
        CommandVariant cmd = this->protocolo.receive_command();

        if (std::holds_alternative<LoginDTO>(cmd)) {
            LoginDTO login_data = std::get<LoginDTO>(cmd);

            JoinEvent joinEvent{this->clientId, login_data.username};
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
            CommandVariant cmd = this->protocolo.receive_command();
            
            PlayerCommand playerCmd{this->clientId, cmd};

            this->gameQueue.push(playerCmd);
        }
    } catch (const std::exception& e) {
        DisconnectEvent disconnect{this->clientId};
        this->gameQueue.push(disconnect);
    }
}

void Receiver::run() {
    if (authenticatePlayer()) {
        inGameCommunication();
    }
}
