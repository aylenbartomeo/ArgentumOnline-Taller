#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <unordered_set>

#include <SDL2/SDL_keycode.h>

struct FrameInput {
    bool quit = false;
    bool moveNorth = false;
    bool moveSouth = false;
    bool moveEast = false;
    bool moveWest = false;
};

class EventHandler {
private:
    std::unordered_set<SDL_Keycode> pressedKeys;
    bool quitRequested = false;

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
