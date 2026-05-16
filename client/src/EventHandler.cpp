#include "EventHandler.h"

#include <SDL2/SDL.h>

FrameInput EventHandler::poll_events() {
    FrameInput input;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            input.quit = true;
        } else if (event.type == SDL_KEYDOWN &&
                   event.key.keysym.sym == SDLK_ESCAPE) {
            input.quit = true;
        }
    }
    return input;
}
