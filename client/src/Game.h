#ifndef GAME_H
#define GAME_H

#include <SDL2pp/SDL2pp.hh>

#include "EventHandler.h"
#include "Window.h"

class Game {
private:
    SDL2pp::SDL sdl;
    Window window;
    EventHandler events;

public:
    Game();
    ~Game() = default;

    void run();

    /* No permito copias */
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    /* Permito movimientos */
    Game(Game&&) = default;
    Game& operator=(Game&&) = default;
};

#endif
