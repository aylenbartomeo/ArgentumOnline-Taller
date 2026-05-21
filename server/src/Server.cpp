#include "Server.h"

Server::Server(const char* port):
        gameQueue(),
        monitor(),
        gameLoop(gameQueue, monitor),
        acceptor(port, gameQueue, monitor, auth) {}

void Server::run() {
    gameLoop.start();
    acceptor.start();
    wait_for_exit();
}

void Server::wait_for_exit() {
    std::string cmd;
    while (std::getline(std::cin, cmd)) {
        if (cmd == EXIT_CMD) {
            break;
        }
    }
}

Server::~Server() {
    try {
        acceptor.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error shutting down acceptor: " << e.what() << std::endl;
    }
    acceptor.join();
    gameLoop.stop();
    gameQueue.close();
    gameLoop.join();
}
