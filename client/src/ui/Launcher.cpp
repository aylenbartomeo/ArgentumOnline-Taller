#include "Launcher.h"

#include <iostream>
#include <utility>

#include <SDL_image.h>

const int WINDOW_W = 800;
const int WINDOW_H = 600;

const SDL_Rect BTN_PROBAR = {530, 305, 200, 40};
const SDL_Rect BTN_COMENZAR = {280, 490, 240, 50};

const SDL_Rect BTN_LOGIN = {450, 320, 200, 40};
const SDL_Rect BTN_REGISTER = {120, 445, 200, 45};
const SDL_Rect BTN_CLOSE_AUTH = {500, 450, 200, 45};
const SDL_Rect BTN_CLOSE_CONNECTION = {760, 8, 30, 30};
Launcher::Launcher(int width, int height, bool fullscreen):
        client(nullptr),
        authenticated(false),
        isOnline(false),
        errorMessage(""),
        sdl(SDL_INIT_VIDEO),
        window("Argentum Online", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
               SDL_WINDOW_SHOWN | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)),
        renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        state(LauncherState::CONNECTION) {

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    renderer.SetLogicalSize(800, 600);

    if (TTF_WasInit() == 0) {
        TTF_Init();
    }
    font = TTF_OpenFont("resources/ui/fonts/DejaVuSans-Bold.ttf", 16);
    if (!font) {
        std::cerr << "[Launcher] Error al cargar fuente: " << TTF_GetError() << std::endl;
    }

    SDL_Texture* tConn = IMG_LoadTexture(renderer.Get(), "resources/ui/pantallas/IP-PUERTO.png");
    if (tConn) {
        bgConnection = std::make_unique<SDL2pp::Texture>(tConn);
    } else {
        std::cerr << "[Launcher] No se pudo cargar IP-PUERTO.png: " << IMG_GetError() << std::endl;
    }

    SDL_Texture* tAuth =
            IMG_LoadTexture(renderer.Get(), "resources/ui/pantallas/REGISTRO-LOGIN.png");
    if (tAuth) {
        bgAuthentication = std::make_unique<SDL2pp::Texture>(tAuth);
    } else {
        std::cerr << "[Launcher] No se pudo cargar REGISTRO-LOGIN.png: " << IMG_GetError()
                  << std::endl;
    }

    ipBox = {155, 251, 340, 35, "127.0.0.1", false, false};
    portBox = {155, 328, 340, 35, "8080", false, false};

    userBox = {122, 245, 340, 35, "", false, false};
    passBox = {415, 245, 340, 35, "", false, true};

    SDL_StartTextInput();
}

Launcher::~Launcher() {
    SDL_StopTextInput();
    if (font)
        TTF_CloseFont(font);
}

void Launcher::run() {
    bool quit = false;
    while (!quit && !authenticated) {
        handleEvents(quit);
        render();
    }
}

bool Launcher::isAuthenticated() const { return authenticated; }

std::unique_ptr<Client> Launcher::releaseClient() { return std::move(client); }

void Launcher::handleEvents(bool& quit) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                handleMouseClick(e.button.x, e.button.y, quit);
            }
        } else if (e.type == SDL_TEXTINPUT) {
            TextBox* activeBox = nullptr;
            if (state == LauncherState::CONNECTION) {
                if (ipBox.isFocused)
                    activeBox = &ipBox;
                if (portBox.isFocused)
                    activeBox = &portBox;
            } else {
                if (userBox.isFocused)
                    activeBox = &userBox;
                if (passBox.isFocused)
                    activeBox = &passBox;
            }
            if (activeBox) {
                std::string newText = activeBox->text + e.text.text;
                std::string display = newText;
                if (activeBox->isPassword) {
                    display = std::string(newText.length(), '*');
                }

                int w = 0, h = 0;
                if (font && TTF_SizeUTF8(font, display.c_str(), &w, &h) == 0) {
                    int max_w = activeBox->w - 30;
                    if (state == LauncherState::CONNECTION) {
                        max_w -= 80;  // Equivalente a ~8 caracteres menos para IP y Puerto
                    } else if (state == LauncherState::AUTHENTICATION) {
                        max_w -= 60;  // Equivalente a ~6 caracteres menos para Usuario y Contraseña
                        if (activeBox->isPassword) {
                            max_w -= 10;  // 1 caracter menos para la contraseña
                        }
                    }
                    if (w <= max_w) {
                        activeBox->text = newText;
                    }
                } else if (newText.length() < 50) {  // Fallback en caso de error
                    activeBox->text = newText;
                }
            }
        } else if (e.type == SDL_KEYDOWN) {
            TextBox* activeBox = nullptr;
            if (state == LauncherState::CONNECTION) {
                if (ipBox.isFocused)
                    activeBox = &ipBox;
                if (portBox.isFocused)
                    activeBox = &portBox;
            } else {
                if (userBox.isFocused)
                    activeBox = &userBox;
                if (passBox.isFocused)
                    activeBox = &passBox;
            }

            if (e.key.keysym.sym == SDLK_BACKSPACE && activeBox && !activeBox->text.empty()) {
                activeBox->text.pop_back();
            } else if (e.key.keysym.sym == SDLK_TAB) {
                if (state == LauncherState::CONNECTION) {
                    if (ipBox.isFocused) {
                        ipBox.isFocused = false;
                        portBox.isFocused = true;
                    } else {
                        ipBox.isFocused = true;
                        portBox.isFocused = false;
                    }
                } else {
                    if (userBox.isFocused) {
                        userBox.isFocused = false;
                        passBox.isFocused = true;
                    } else {
                        userBox.isFocused = true;
                        passBox.isFocused = false;
                    }
                }
            }
        }
    }
}

void Launcher::handleMouseClick(int x, int y, bool& quit) {
    auto isInside = [](int mx, int my, const SDL_Rect& r) {
        return mx >= r.x && mx <= (r.x + r.w) && my >= r.y && my <= (r.y + r.h);
    };
    auto isInsideBox = [](int mx, int my, const TextBox& b) {
        return mx >= b.x && mx <= (b.x + b.w) && my >= b.y && my <= (b.y + b.h);
    };

    if (state == LauncherState::CONNECTION) {
        if (isInside(x, y, BTN_CLOSE_CONNECTION)) {
            quit = true;
            return;
        }

        ipBox.isFocused = isInsideBox(x, y, ipBox);
        portBox.isFocused = isInsideBox(x, y, portBox);

        if (isInside(x, y, BTN_PROBAR)) {
            try {
                client = std::make_unique<Client>(ipBox.text.c_str(), portBox.text.c_str());
                isOnline = true;
                errorMessage = "";
            } catch (const std::exception& e) {
                client.reset();
                isOnline = false;
                errorMessage = "IP y PUERTO sin conexión. Intente con otra dirección";
            }
        } else if (isInside(x, y, BTN_COMENZAR)) {
            if (isOnline) {
                state = LauncherState::AUTHENTICATION;
                errorMessage = "";
            } else {
                errorMessage = "Debe conectar con un server EN LINEA";
            }
        }
    } else if (state == LauncherState::AUTHENTICATION) {
        if (isInside(x, y, BTN_CLOSE_AUTH)) {
            quit = true;
            return;
        }

        userBox.isFocused = isInsideBox(x, y, userBox);
        passBox.isFocused = isInsideBox(x, y, passBox);

        if (isInside(x, y, BTN_LOGIN)) {
            if (client && client->authenticate("login", userBox.text, passBox.text, errorMessage)) {
                authenticated = true;
            }
        } else if (isInside(x, y, BTN_REGISTER)) {
            if (client &&
                client->authenticate("register", userBox.text, passBox.text, errorMessage)) {
                authenticated = true;
            }
        }
    }
}

void Launcher::render() {
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    if (state == LauncherState::CONNECTION) {
        if (bgConnection) {
            renderer.Copy(*bgConnection);
        }
        renderTextBox(ipBox);
        renderTextBox(portBox);

        std::string statusText = isOnline ? "EN LINEA" : "DESCONECTADO";
        SDL_Color color = isOnline ? SDL_Color{0, 255, 0, 255} : SDL_Color{255, 0, 0, 255};
        renderTextCenter(statusText, 640, 250, color);

        if (!errorMessage.empty()) {
            renderTextCenter(errorMessage, 400, 560, {220, 180, 50, 255});
        }
    } else {
        if (bgAuthentication) {
            renderer.Copy(*bgAuthentication);
        }
        renderTextBox(userBox);
        renderTextBox(passBox);

        if (!errorMessage.empty()) {
            renderTextCenter(errorMessage, 400, 570, {220, 180, 50, 255});
        }
    }

    renderer.Present();
}

void Launcher::renderTextBox(const TextBox& box) {
    std::string display = box.text;
    if (box.isPassword) {
        display = std::string(box.text.length(), '*');
    }

    if (box.isFocused) {
        if ((SDL_GetTicks() / 500) % 2 == 0) {
            display += "|";
        } else {
            display += " ";
        }
    }

    if (display.empty())
        return;

    if (font) {
        SDL_Color color = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderUTF8_Blended(font, display.c_str(), color);
        if (surf) {
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer.Get(), surf);
            if (t) {
                SDL2pp::Texture tex(t);
                int textX = box.x + 15;                     // Left aligned con un pequeño margen
                int textY = box.y + (box.h - surf->h) / 2;  // Centrado vertical
                SDL_Rect dst = {textX, textY, surf->w, surf->h};
                renderer.Copy(tex, SDL2pp::NullOpt, dst);
            }
            SDL_FreeSurface(surf);
        }
    }
}

void Launcher::renderTextCenter(const std::string& text, int x, int y, SDL_Color color) {
    if (!font || text.empty())
        return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (surf) {
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer.Get(), surf);
        if (t) {
            SDL2pp::Texture tex(t);
            SDL_Rect dst = {x - surf->w / 2, y, surf->w, surf->h};
            renderer.Copy(tex, SDL2pp::NullOpt, dst);
        }
        SDL_FreeSurface(surf);
    }
}
