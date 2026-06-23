#ifndef QUIT_VIEW_H
#define QUIT_VIEW_H

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../rendering/TextureManager.h"

enum class QuitViewAction {
    None,
    Quit,   // Confirmó salir (Equivale a hacer clic en ACEPTAR)
    Resume  // Eligió volver al juego (Equivale a hacer clic en VOLVER o ESC)
};

class QuitView {
public:
    QuitView();
    ~QuitView() = default;

    // Ejecuta el bucle propio de la ventana de salida de forma bloqueante
    QuitViewAction run();

private:
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    TextureManager textures;  // Instancia local para esta ventana independiente

    // Hitboxes absolutas idénticas a las posiciones del Launcher
    static const SDL_Rect BTN_VOLVER;   // Ubicación de BTN_REGISTER {120, 445, 200, 45}
    static const SDL_Rect BTN_ACEPTAR;  // Ubicación de BTN_CLOSE    {500, 450, 200, 45}

    bool pointInRect(int x, int y, const SDL_Rect& rect);
};

#endif  // QUIT_VIEW_H
