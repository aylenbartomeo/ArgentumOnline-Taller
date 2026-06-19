#include "Window.h"

#include <SDL2/SDL.h>

Window::Window(const std::string& title, int width, int height, bool fullscreen):
        window(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
               SDL_WINDOW_RESIZABLE | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)),
        renderer(window, -1, SDL_RENDERER_ACCELERATED) {}

SDL2pp::Renderer& Window::getRenderer() { return renderer; }

int Window::getWidth() { return window.GetWidth(); }

int Window::getHeight() { return window.GetHeight(); }
