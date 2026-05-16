#include "Window.h"

#include <SDL2/SDL.h>

Window::Window(const std::string& title, int width, int height)
    : window(title,
             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
             width, height,
             SDL_WINDOW_RESIZABLE)
    , renderer(window, -1, SDL_RENDERER_ACCELERATED) {}

void Window::clear() {
    renderer.Clear();
}

void Window::present() {
    renderer.Present();
}
