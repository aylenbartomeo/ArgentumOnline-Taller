#include "QuitView.h"

// Asignamos las hitboxes clonando las posiciones exactas del Launcher
const SDL_Rect QuitView::BTN_VOLVER = {120, 445, 200, 45};   // Mismo que BTN_REGISTER
const SDL_Rect QuitView::BTN_ACEPTAR = {500, 450, 200, 45};  // Mismo que BTN_CLOSE

QuitView::QuitView():
        window("Confirmar Salida", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
               SDL_WINDOW_SHOWN),
        renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        textures(renderer) {}  // El TextureManager solo se inicializa con el renderer

QuitViewAction QuitView::run() {
    bool running = true;
    QuitViewAction action = QuitViewAction::None;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                action = QuitViewAction::Quit;
                running = false;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    // Presionar ESC cancela y vuelve al juego
                    action = QuitViewAction::Resume;
                    running = false;
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    int mx = e.button.x;
                    int my = e.button.y;

                    if (pointInRect(mx, my, BTN_VOLVER)) {
                        action = QuitViewAction::Resume;
                        running = false;
                    } else if (pointInRect(mx, my, BTN_ACEPTAR)) {
                        action = QuitViewAction::Quit;
                        running = false;
                    }
                }
            }
        }

        // Renderizado limpio de la ventana
        renderer.Clear();

        // CORRECCIÓN: Le pedimos la textura cargada/cacheada al TextureManager
        renderer.Copy(textures.get("resources/ui/pantallas/SALIR.png"), SDL2pp::NullOpt,
                      SDL2pp::NullOpt);

        renderer.Present();

        // Control de FPS para el bucle (60 FPS aproximadamente)
        SDL_Delay(16);
    }

    // Al salir del scope, las variables de SDL2pp se destruyen de forma segura por RAII
    return action;
}

bool QuitView::pointInRect(int x, int y, const SDL_Rect& rect) {
    return (x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h);
}
