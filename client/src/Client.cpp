#include "Client.h"

#include <iostream>
#include <string>

#include "common/include/dto/LoginDTO.h"
#include "common/include/dto/RegisterDTO.h"

Client::Client(const char* hostname, const char* servname, const char* username):
        clientId(0),
        username(username),
        skt(hostname, servname),
        protocol(skt),
        snapshotQueue(),
        commandQueue(),
        receiver(protocol, *this),
        sender(protocol, commandQueue),
        wasStarted(false) {}

bool Client::authenticate(const std::string& action, const std::string& username,
                          const std::string& password) {
    if (action == "login") {
        LoginDTO dto(username, password);
        protocol.send_login(dto);

        LoginResponseDTO response = protocol.recv_login_response();
        if (response.success) {
            std::cout << "[CLIENT] Login successful. Entering the world...\n";
            this->clientId = response.clientId;
            return true;
        } else {
            std::cerr << "[CLIENT] Login error: " << response.errorMessage << "\n";
            return false;
        }

    } else if (action == "register") {
        RegisterDTO dto(username, password);
        protocol.send_register(dto);
        LoginResponseDTO response = protocol.recv_register_response();
        if (response.success) {
            std::cout << "[CLIENT] Registration successful. Entering the world...\n";
            this->clientId = response.clientId;
            return true;
        } else {
            std::cerr << "[CLIENT] Registration error: " << response.errorMessage << "\n";
            return false;
        }
    }

    std::cerr << "[CLIENT] Unknown action.\n";
    return false;
}

void Client::start() {
    receiver.start();
    sender.start();
    wasStarted = true;
    std::cout << "[CLIENT] Connected as " << this->username << " (id=" << this->clientId << ")"
              << std::endl;
}

void Client::stop() {
    if (!wasStarted)
        return;
    try {
        skt.shutdown(SHUT_RDWR);
        skt.close();
    } catch (...) {}
    snapshotQueue.close();
    commandQueue.close();
    receiver.stop();
    sender.stop();
    receiver.join();
    sender.join();
    wasStarted = false;
}

void Client::pushSnapshot(const SnapshotDTO& snap) {
    try {
        snapshotQueue.push(snap);
    } catch (...) {}
}

bool Client::tryPopSnapshot(SnapshotDTO& out) { return snapshotQueue.try_pop(out); }

void Client::sendCommand(const CommandVariant& cmd) {
    try {
        commandQueue.push(cmd);
    } catch (...) {}
}

uint32_t Client::getClientId() const { return clientId; }

Client::~Client() { stop(); }
