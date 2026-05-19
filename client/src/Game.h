#ifndef GAME_H
#define GAME_H

#include <SDL2pp/SDL2pp.hh>

#include "Client.h"
#include "EventHandler.h"
#include "Window.h"

class Game {
private:
    SDL2pp::SDL sdl;
    Window window;
    EventHandler events;
    Client& client;

public:
    explicit Game(Client& client);
    ~Game() = default;

    void run();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    Game(Game&&) = default;
    Game& operator=(Game&&) = default;

private:
    void render();
};

#endif
