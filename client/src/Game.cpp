#include "Game.h"

#include <SDL2/SDL.h>

Game::Game()
    : sdl(SDL_INIT_VIDEO)
    , window("Argentum Online - Cliente", 640, 480)
    , events() {}

void Game::run() {
    while (true) {
        FrameInput input = events.poll_events();
        if (input.quit) {
            break;
        }
        window.clear();
        window.present();
        SDL_Delay(16);
    }
}
