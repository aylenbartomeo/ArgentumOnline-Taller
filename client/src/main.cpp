#include <exception>
#include <iostream>

#include "Client.h"
#include "Game.h"

int main(int argc, char* argv[]) try {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <username>" << std::endl;
        return 1;
    }
    Client client("localhost", "8080", argv[1]);
    client.start();
    Game game(client);
    game.run();
    client.stop();
    return 0;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
