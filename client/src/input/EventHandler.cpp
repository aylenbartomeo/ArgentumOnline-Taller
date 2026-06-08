#include "EventHandler.h"

#include <SDL2/SDL.h>

FrameInput EventHandler::pollEvents() {
    SDL_Event event;
    bool attackThisFrame = false;
    int attackX = 0;
    int attackY = 0;
    bool equipThisFrame = false;
    int equipX = 0;
    int equipY = 0;
    bool resurrectThisFrame = false;

    bool toggleChatThisFrame = false;
    int scrollThisFrame = 0;
    bool mouseLeftJustPressedThisFrame = false;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quitRequested = true;

        } else if (event.type == SDL_MOUSEMOTION) {
            currentMouseX = event.motion.x;
            currentMouseY = event.motion.y;

        } else if (event.type == SDL_MOUSEWHEEL) {
            if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
                scrollThisFrame -= event.wheel.y;
            } else {
                scrollThisFrame += event.wheel.y;
            }

        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                currentMouseX = event.button.x;
                currentMouseY = event.button.y;
                mouseLeftHeld = true;
                mouseLeftJustPressedThisFrame = true;

                if (!inputActive) {
                    if (event.button.clicks == 2) {
                        equipThisFrame = true;
                        equipX = event.button.x;
                        equipY = event.button.y;
                    } else {
                        attackThisFrame = true;
                        attackX = event.button.x;
                        attackY = event.button.y;
                    }
                }
            }

        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseLeftHeld = false;
            }

        } else if (event.type == SDL_KEYDOWN) {
            const SDL_Keycode key = event.key.keysym.sym;

            if (key == SDLK_F2 && event.key.repeat == 0) {
                toggleChatThisFrame = true;
            }

            if (inputActive) {
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    inputActive = false;
                    SDL_StopTextInput();
                } else if (key == SDLK_BACKSPACE && !inputBuffer.empty()) {
                    inputBuffer.pop_back();
                } else if (key == SDLK_ESCAPE) {
                    inputBuffer.clear();
                    inputActive = false;
                    SDL_StopTextInput();
                }
            } else {
                if (key == SDLK_ESCAPE) {
                    quitRequested = true;
                } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    inputActive = true;
                    SDL_StartTextInput();
                } else if (key == SDLK_r) {
                    if (event.key.repeat == 0)
                        resurrectThisFrame = true;
                }
                pressedKeys.insert(key);
                justPressedKeys.insert(key);
                justPressedScancodes.insert(event.key.keysym.scancode);
            }
        } else if (event.type == SDL_KEYUP) {
            pressedKeys.erase(event.key.keysym.sym);
        } else if (event.type == SDL_TEXTINPUT && inputActive) {
            inputBuffer += event.text.text;
        }
    }

    FrameInput input;
    input.quit = quitRequested;

    input.mouseX = currentMouseX;
    input.mouseY = currentMouseY;
    input.mouseLeftHeld = mouseLeftHeld;
    input.mouseLeftJustPressed = mouseLeftJustPressedThisFrame;
    input.mouseScroll = scrollThisFrame;
    input.toggleChat = toggleChatThisFrame;

    if (inputActive) {
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

        bool shiftHeld = (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
        input.cheatLevelUp = shiftHeld && justPressedScancodes.count(SDL_SCANCODE_L);
        input.cheatDie = shiftHeld && justPressedScancodes.count(SDL_SCANCODE_K);
        input.cheatGiveGold = shiftHeld && justPressedScancodes.count(SDL_SCANCODE_G);
    }

    input.attackPressed = attackThisFrame;
    input.attackX = attackX;
    input.attackY = attackY;
    input.equipPressed = equipThisFrame;
    input.equipX = equipX;
    input.equipY = equipY;
    input.resurrectPressed = resurrectThisFrame;

    justPressedKeys.clear();
    justPressedScancodes.clear();
    return input;
}
