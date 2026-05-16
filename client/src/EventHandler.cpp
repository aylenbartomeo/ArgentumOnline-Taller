#include "EventHandler.h"

#include <SDL2/SDL.h>

FrameInput EventHandler::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quitRequested = true;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                quitRequested = true;
            }
            pressedKeys.insert(event.key.keysym.sym);
        } else if (event.type == SDL_KEYUP) {
            pressedKeys.erase(event.key.keysym.sym);
        }
    }

    FrameInput input;
    input.quit = quitRequested;
    input.moveNorth = pressedKeys.count(SDLK_w) || pressedKeys.count(SDLK_UP);
    input.moveSouth = pressedKeys.count(SDLK_s) || pressedKeys.count(SDLK_DOWN);
    input.moveEast  = pressedKeys.count(SDLK_d) || pressedKeys.count(SDLK_RIGHT);
    input.moveWest  = pressedKeys.count(SDLK_a) || pressedKeys.count(SDLK_LEFT);
    return input;
}
