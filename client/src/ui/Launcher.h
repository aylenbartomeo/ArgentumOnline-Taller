#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <memory>
#include <string>

#include <SDL2pp/SDL2pp.hh>
#include <SDL_ttf.h>

#include "../core/Client.h"

enum class LauncherState { CONNECTION, AUTHENTICATION };

struct TextBox {
    int x, y, w, h;
    std::string text;
    bool isFocused;
    bool isPassword;
};

class Launcher {
public:
    Launcher(int width, int height, bool fullscreen);
    ~Launcher();

    void run();

    bool isAuthenticated() const;
    std::unique_ptr<Client> releaseClient();

private:
    std::unique_ptr<Client> client;
    bool authenticated;
    bool isOnline;
    std::string errorMessage;

    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;

    TTF_Font* font;
    std::unique_ptr<SDL2pp::Texture> bgConnection;
    std::unique_ptr<SDL2pp::Texture> bgAuthentication;

    LauncherState state;

    TextBox ipBox;
    TextBox portBox;
    TextBox userBox;
    TextBox passBox;

    void handleEvents(bool& quit);
    void handleMouseClick(int x, int y, bool& quit);
    void render();

    void renderTextBox(const TextBox& box);
    void renderTextCenter(const std::string& text, int x, int y, SDL_Color color);
};

#endif
