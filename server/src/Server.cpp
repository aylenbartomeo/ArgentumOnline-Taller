#include "Server.h"

Server::Server(const char* port): acceptor(port) {}

void Server::start_acceptor() { acceptor.start(); }

void Server::wait_for_exit() {
    std::string cmd;
    while (getline(std::cin, cmd)) {
        if (cmd == EXIT_CMD) {
            break;
        }
    }
}

void Server::run() {
    start_acceptor();
    wait_for_exit();
}

Server::~Server() {
    try {
        acceptor.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error apagando acceptor: " << e.what() << std::endl;
    }
    acceptor.join();
}
