#ifndef WINDOW_H
#define WINDOW_H

#include <string>

#include <SDL2pp/SDL2pp.hh>

class Window {
private:
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;

public:
    Window(const std::string& title, int width, int height);
    ~Window() = default;

    void clear();
    void present();

    /* No permito copias */
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /* Permito movimientos */
    Window(Window&&) = default;
    Window& operator=(Window&&) = default;
};

#endif
