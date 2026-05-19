#include <exception>
#include <iostream>

#include "Client.h"
#include "Game.h"

int main() try {
    Client client("localhost", "8080", "Aylu");
    client.start();
    Game game(client);
    game.run();
    client.stop();
    return 0;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
