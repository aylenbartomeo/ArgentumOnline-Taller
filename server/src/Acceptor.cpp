#include "Acceptor.h"

#include <iostream>

#define EXE_KNOWN_MSG "Error en Acceptor: "

Acceptor::Acceptor(const char* port, Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor):
        skt(port), gameQueue(gameQueue), monitor(monitor) {}

void Acceptor::run() {
    while (should_keep_running()) {
        try {
            Socket client_skt = skt.accept();
            uint32_t client_id = next_client_id++;
            ClientHandler* client = new ClientHandler(std::move(client_skt), client_id, gameQueue);
            this->monitor.addClient(client_id, &(client->getSenderQueue()));
            client->startThreads();
            clients.push_back(client);
            reap();
        } catch (const std::exception& e) {
            if (should_keep_running()) {
                std::cerr << EXE_KNOWN_MSG << e.what() << std::endl;
            }
        }
    }
    close_clients();
}

void Acceptor::reap() {
    clients.remove_if([this](ClientHandler* client) {
        if (!client->isOnline()) {
            this->monitor.removeClient(client->getId());
            client->stopThreads();
            client->joinThreads();
            delete client;
            return true;
        }
        return false;
    });
}

void Acceptor::close_clients() {
    for (ClientHandler* client: clients) {
        this->monitor.removeClient(client->getId());
        client->stopThreads();
        client->joinThreads();
        delete client;
    }
    clients.clear();
}

void Acceptor::stop() {
    Thread::stop();
    try {
        skt.shutdown(SHUT_RDWR);
        skt.close();
    } catch (...) {}
}
