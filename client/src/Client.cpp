#include "Client.h"

#include <iostream>

#include "common/include/dto/LoginDTO.h"

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

void Client::start() {
    LoginDTO loginDTO(this->username, "1234");
    protocol.send_login(loginDTO);

    LoginResponseDTO response = protocol.recv_login_response();

    if (!response.success){
        std::cerr << "[CLIENT] Error de autenticación: " << response.errorMessage << std::endl;
        throw std::runtime_error("Fallo el login: " + response.errorMessage);
    }
    
    this->clientId = response.clientId;
    receiver.start();
    sender.start();
    wasStarted = true;
    std::cout << "[CLIENT] connected as " << this->username
              << " (id=" << this->clientId << ")" << std::endl;
}

void Client::stop() {
    if (!wasStarted) return;
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

bool Client::tryPopSnapshot(SnapshotDTO& out) {
    return snapshotQueue.try_pop(out);
}

void Client::sendCommand(const CommandVariant& cmd) {
    try {
        commandQueue.push(cmd);
    } catch (...) {}
}

uint32_t Client::getClientId() const {
    return clientId;
}

Client::~Client() {
    stop();
}
