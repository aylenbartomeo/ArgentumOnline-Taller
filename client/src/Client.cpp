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
    receiver.start();
    sender.start();
    wasStarted = true;
    std::cout << "[CLIENTE] conectado como " << this->username << std::endl;
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

Client::~Client() {
    stop();
}
