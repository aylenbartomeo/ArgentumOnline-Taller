#include "ClientHandler.h"

#include <utility>

ClientHandler::ClientHandler(Socket&& skt, uint32_t id, Queue<GameEvent>& gameQueue):
        client_id(id),
        skt(std::move(skt)),
        sender_queue(),
        receiver(this->skt, this->client_id, gameQueue),
        sender(this->skt, this->sender_queue) {}

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

uint32_t ClientHandler::getId() const { return client_id; }
