#include <exception>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

int main() try {
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);

    SDL2pp::Window window(
        "Argentum Online - Cliente",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_RESIZABLE);

    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool running = true;
    SDL_Event event;

    while (running) {
        // 1) Procesar eventos SDL acumulados desde el último frame
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN &&
                       event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        // 2) Render del frame (pantalla negra por ahora)
        renderer.Clear();
        renderer.Present();

        // 3) Frame pacing: ~60 fps, evita ocupar 100% CPU
        SDL_Delay(16);
    }

    return 0;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
