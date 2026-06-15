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
                this->protocolo.sendLoginFailed("El usuario no puede estar vacío.");
                return false;
            }

            auto authResult = this->auth.validateUser(login_data.username, login_data.password);
            if (authResult.has_value()) {

                uint32_t targetId = authResult.value();

                if (this->monitor.isClientConnected(targetId)) {
                    this->protocolo.sendLoginFailed("El usuario ya está conectado en otra sesión.");
                    return false;
                }

                this->clientId = targetId;
                std::cout << "[SERVER] Jugador autenticado: " << login_data.username
                          << " (id=" << this->clientId << ")" << std::endl;

                this->protocolo.sendLoginSuccess(this->clientId);
                this->monitor.addClient(this->clientId, &this->senderQueue);
                JoinEvent joinEvent{this->clientId, login_data.username};
                this->gameQueue.push(joinEvent);
                return true;
            } else {
                this->protocolo.sendLoginFailed("Contraseña incorrecta o el usuario no existe.");
                return false;
            }
        } else if (std::holds_alternative<RegisterDTO>(cmd)) {
            RegisterDTO register_data = std::get<RegisterDTO>(cmd);
            if (register_data.username.empty()) {
                this->protocolo.sendRegisterFailed("El usuario no puede estar vacío.");
                return false;
            }

            auto authResult =
                    this->auth.registerUser(register_data.username, register_data.password);
            if (authResult.has_value()) {
                this->clientId = authResult.value();
                std::cout << "[SERVER] Nuevo jugador registrado: " << register_data.username
                          << " (id=" << this->clientId << ")" << std::endl;

                this->protocolo.sendRegisterSuccess(this->clientId);
                this->monitor.addClient(this->clientId, &this->senderQueue);
                JoinEvent joinEvent{this->clientId, register_data.username};
                this->gameQueue.push(joinEvent);
                return true;
            } else {
                this->protocolo.sendRegisterFailed("El usuario ya existe.");
                return false;
            }
        } else {
            return false;
        }
    } catch (const std::exception& e) {
        std::string errStr = e.what();
        if (errStr == "Unknown command received in-game" ||
            errStr == "Comando desconocido recibido en el juego") {
            std::cout << "[SERVIDOR] Cliente desconectado en pantalla de autenticación."
                      << std::endl;
        } else {
            std::cerr << "[SERVIDOR] Error en autenticación: " << e.what() << std::endl;
        }
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
