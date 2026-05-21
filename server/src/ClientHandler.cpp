#include "ClientHandler.h"

#include <utility>

ClientHandler::ClientHandler(Socket&& skt, Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor,
                             AuthManager& auth):
        skt(std::move(skt)),
        sender_queue(),
        receiver(this->skt, gameQueue, monitor, auth, this->sender_queue),
        sender(this->skt, this->sender_queue) {}

uint32_t ClientHandler::getId() const { return receiver.getClientId(); }

void ClientHandler::startThreads() {
    receiver.start();
    sender.start();
}

void ClientHandler::stopThreads() {
    sender_queue.close();
    try {
        skt.shutdown(SHUT_RDWR);
        skt.close();
    } catch (...) {}
}

void ClientHandler::joinThreads() {
    receiver.join();
    sender.join();
}

bool ClientHandler::isOnline() const { return receiver.is_alive() && sender.is_alive(); }

Queue<SnapshotDTO>& ClientHandler::getSenderQueue() { return sender_queue; }
