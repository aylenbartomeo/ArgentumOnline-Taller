#include "Receiver.h"

#include <iostream>

Receiver::Receiver(Socket& skt, uint32_t clientId, Queue<GameEvent>& gameQueue):
        skt(skt), clientId(clientId), gameQueue(gameQueue), protocolo(skt) {}

bool Receiver::authenticatePlayer() {
    try {
        CommandVariant cmd = this->protocolo.receive_command();

        if (std::holds_alternative<LoginDTO>(cmd)) {
            LoginDTO login_data = std::get<LoginDTO>(cmd);

            // Logica de validacion (hardcodeado)
            if (login_data.password != "1234") {
                this->protocolo.send_login_failed("Contraseña incorrecta. Intente nuevamente.");
                return false;
            }

            if (login_data.username.empty()) {
                this->protocolo.send_login_failed("El nombre de usuario no puede estar vacío.");
                return false;
            }

            std::cout << "[SERVER] Jugador autenticado exitosamente: " << login_data.username
                      << std::endl;

            JoinEvent joinEvent{this->clientId, login_data.username};
            this->gameQueue.push(joinEvent);
            this->protocolo.send_login_success(this->clientId);
            return true;
        } else {
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "[SERVER] Error durante la autenticación del cliente " << this->clientId
                  << ": " << e.what() << std::endl;
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

void Receiver::run() {
    if (authenticatePlayer()) {
        inGameCommunication();
    }
}
