#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <string>
#include <unordered_set>

#include <SDL2/SDL_keycode.h>

struct FrameInput {
    bool quit = false;
    bool moveNorth = false;
    bool moveSouth = false;
    bool moveEast = false;
    bool moveWest = false;

    // --- Minichat ---
    bool chatInputActive = false;  // el jugador está escribiendo en el chat
    bool chatSubmitted = false;    // presionó Enter con texto
    std::string chatText;          // texto acumulado del input actual

    // --- Cheats ---
    bool cheatLevelUp = false;
    bool cheatDie = false;
};

class EventHandler {
private:
    std::unordered_set<SDL_Keycode> pressedKeys;
    bool quitRequested = false;

    bool inputActive = false;
    std::string inputBuffer;

    std::unordered_set<SDL_Keycode> justPressedKeys;
    std::unordered_set<SDL_Scancode> justPressedScancodes;

public:
    EventHandler() = default;
    ~EventHandler() = default;

    FrameInput pollEvents();

    /* No permito copias */
    EventHandler(const EventHandler&) = delete;
    EventHandler& operator=(const EventHandler&) = delete;

    /* Permito movimientos */
    EventHandler(EventHandler&&) = default;
    EventHandler& operator=(EventHandler&&) = default;
};

#endif
