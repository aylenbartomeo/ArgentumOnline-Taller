#include "Client.h"

#include <iostream>

#include "common/include/dto/LoginDTO.h"

Client::Client(const char* hostname, const char* servname, const char* username):
        clientId(0),
        username(username),
        skt(hostname, servname),
        protocol(skt),
        latestSnapshot(),
        receiver(protocol, *this),
        wasStarted(false) {}

void Client::start() {
    LoginDTO loginDTO(this->username, "1234");
    protocol.send_login(loginDTO);
    receiver.start();
    wasStarted = true;
    std::cout << "[CLIENTE] conectado como " << this->username << std::endl;
}

void Client::stop() {
    if (!wasStarted) return;
    try {
        skt.shutdown(SHUT_RDWR);
        skt.close();
    } catch (...) {}
    receiver.stop();
    receiver.join();
    wasStarted = false;
}

void Client::updateSnapshot(const SnapshotDTO& snap) {
    std::lock_guard<std::mutex> lock(snapshotMutex);
    latestSnapshot = snap;
}

SnapshotDTO Client::getLatestSnapshot() {
    std::lock_guard<std::mutex> lock(snapshotMutex);
    return latestSnapshot;
}

Client::~Client() {
    stop();
}
