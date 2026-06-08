#include "Server.h"

Server::Server(const char* port, const WorldConfig& worldConfig):
        gameQueue(),
        monitor(),
        gameLoop(gameQueue, monitor, "config", worldConfig),
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
    try {
        gameQueue.close();
    } catch (const std::exception& e) {
        std::cerr << "Error closing game queue: " << e.what() << std::endl;
    }
    gameLoop.join();
}
