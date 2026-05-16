#include <exception>
#include <iostream>

#include "Game.h"

int main() try {
    Game game;
    game.run();
    return 0;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
