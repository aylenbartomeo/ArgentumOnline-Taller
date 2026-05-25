#include "Receiver.h"

#include <iostream>

#include "../../common/include/dto/RegisterDTO.h"
#include "auth/AuthManager.h"

Receiver::Receiver(Socket& skt, Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor,
                   AuthManager& authManager, Queue<ServerMessageVariant>& senderQueue):
        skt(skt),
        clientId(0),
        gameQueue(gameQueue),
        protocolo(skt),
        monitor(monitor),
        auth(authManager),
        senderQueue(senderQueue) {}

bool Receiver::authenticatePlayer() {
    try {
        CommandVariant cmd = this->protocolo.receive_command();
        if (std::holds_alternative<LoginDTO>(cmd)) {
            LoginDTO login_data = std::get<LoginDTO>(cmd);
            if (login_data.username.empty()) {
                this->protocolo.send_login_failed("The username cannot be empty.");
                return false;
            }

            auto authResult = this->auth.validateUser(login_data.username, login_data.password);
            if (authResult.has_value()) {

                uint32_t targetId = authResult.value();

                if (this->monitor.isClientConnected(targetId)) {
                    this->protocolo.send_login_failed(
                            "The user is already connected in another session.");
                    return false;
                }

                this->clientId = targetId;
                std::cout << "[SERVER] Authenticated player: " << login_data.username
                          << " (id=" << this->clientId << ")" << std::endl;

                this->protocolo.send_login_success(this->clientId);
                this->monitor.addClient(this->clientId, &this->senderQueue);
                JoinEvent joinEvent{this->clientId, login_data.username};
                this->gameQueue.push(joinEvent);
                return true;
            } else {
                this->protocolo.send_login_failed("Incorrect password or user does not exist.");
                return false;
            }
        } else if (std::holds_alternative<RegisterDTO>(cmd)) {
            RegisterDTO register_data = std::get<RegisterDTO>(cmd);
            if (register_data.username.empty()) {
                this->protocolo.send_register_failed("The username cannot be empty.");
                return false;
            }

            auto authResult =
                    this->auth.registerUser(register_data.username, register_data.password);
            if (authResult.has_value()) {
                this->clientId = authResult.value();
                std::cout << "[SERVER] New registered player: " << register_data.username
                          << " (id=" << this->clientId << ")" << std::endl;

                this->protocolo.send_register_success(this->clientId);
                this->monitor.addClient(this->clientId, &this->senderQueue);
                JoinEvent joinEvent{this->clientId, register_data.username};
                this->gameQueue.push(joinEvent);
                return true;
            } else {
                this->protocolo.send_register_failed("The user already exists.");
                return false;
            }
        } else {
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "[SERVER] Error during authentication: " << e.what() << std::endl;
        this->stop();
        return false;
    }
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

uint32_t Receiver::getClientId() const { return this->clientId; }

void Receiver::run() {
    while (should_keep_running()) {
        if (authenticatePlayer()) {
            inGameCommunication();
            break;
        }
    }
}
