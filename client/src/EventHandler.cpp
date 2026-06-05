#include "EventHandler.h"

#include <SDL2/SDL.h>

FrameInput EventHandler::pollEvents() {
    SDL_Event event;
    bool attackThisFrame = false;
    int attackX = 0;
    int attackY = 0;
    bool resurrectThisFrame = false;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quitRequested = true;

        } else if (event.type == SDL_KEYDOWN) {
            const SDL_Keycode key = event.key.keysym.sym;

            if (inputActive) {
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    // El jugador confirmó el mensaje: salir de modo escritura
                    inputActive = false;
                    SDL_StopTextInput();
                } else if (key == SDLK_BACKSPACE && !inputBuffer.empty()) {
                    // Borrar último carácter
                    inputBuffer.pop_back();
                } else if (key == SDLK_ESCAPE) {
                    // Cancelar sin enviar
                    inputBuffer.clear();
                    inputActive = false;
                    SDL_StopTextInput();
                }
                // Mientras se escribe no se procesan movimientos
            } else {
                if (key == SDLK_ESCAPE) {
                    quitRequested = true;
                } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    // Activar modo escritura
                    inputActive = true;
                    SDL_StartTextInput();
                } else if (key == SDLK_r) {
                    if (event.key.repeat == 0) {
                        resurrectThisFrame = true;
                    }
                } else {
                    pressedKeys.insert(key);
                }
            }

        } else if (event.type == SDL_KEYUP) {
            pressedKeys.erase(event.key.keysym.sym);

        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT && !inputActive) {
                attackThisFrame = true;
                attackX = event.button.x;
                attackY = event.button.y;
            }

        } else if (event.type == SDL_TEXTINPUT && inputActive) {
            inputBuffer += event.text.text;
        }
    }

    FrameInput input;
    input.quit = quitRequested;

    if (inputActive) {
        // Mientras escribe, bloquear movimiento
        input.chatInputActive = true;
        input.chatText = inputBuffer;
    } else {
        input.moveNorth = pressedKeys.count(SDLK_w) || pressedKeys.count(SDLK_UP);
        input.moveSouth = pressedKeys.count(SDLK_s) || pressedKeys.count(SDLK_DOWN);
        input.moveEast = pressedKeys.count(SDLK_d) || pressedKeys.count(SDLK_RIGHT);
        input.moveWest = pressedKeys.count(SDLK_a) || pressedKeys.count(SDLK_LEFT);

        if (!inputBuffer.empty()) {
            input.chatSubmitted = true;
            input.chatText = inputBuffer;
            inputBuffer.clear();
        }
    }

    input.attackPressed = attackThisFrame;
    input.attackX = attackX;
    input.attackY = attackY;
    input.resurrectPressed = resurrectThisFrame;

    return input;
}
