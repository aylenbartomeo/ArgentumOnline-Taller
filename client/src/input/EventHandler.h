#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <string>
#include <unordered_set>

#include <SDL2/SDL_keycode.h>

struct FrameInput {
    // --- Movimiento ---
    bool quit = false;
    bool escPressed = false;
    bool moveNorth = false;
    bool moveSouth = false;
    bool moveEast = false;
    bool moveWest = false;

    // --- Minichat ---
    bool chatInputActive = false;
    bool chatSubmitted = false;
    std::string chatText;
    bool toggleChat = false;
    int mouseScroll = 0;
    int mouseX = 0;
    int mouseY = 0;
    bool mouseLeftHeld = false;
    bool mouseLeftJustPressed = false;

    // --- Combat ---
    bool attackPressed = false;
    int attackX = 0;
    int attackY = 0;
    bool resurrectPressed = false;

    // --- Inventario ---
    bool equipPressed = false;
    int equipX = 0;
    int equipY = 0;
    bool consumeKeyPressed = false;
    bool grabKeyPressed = false;
    bool dropKeyPressed = false;
    bool meditateKeyPressed = false;

    // --- Cheats ---
    bool cheatLevelUp = false;
    bool cheatDie = false;
    bool cheatGiveRanged = false;
    bool cheatInfiniteMana = false;
    bool cheatInfiniteHealth = false;
    bool cheatGiveGold = false;
    bool cheatGiveArmors = false;
    bool cheatGivePotions = false;

    // --- Shoot ---
    bool shootPressed = false;
    int shootScreenX = 0;
    int shootScreenY = 0;

    // --- In game ---
    bool toggleMute = false;
};

class EventHandler {
private:
    std::unordered_set<SDL_Keycode> pressedKeys;
    bool quitRequested = false;

    bool inputActive = false;
    std::string inputBuffer;

    std::unordered_set<SDL_Keycode> justPressedKeys;
    std::unordered_set<SDL_Scancode> justPressedScancodes;

    // --- Estado del mouse ---
    bool mouseLeftHeld = false;
    int currentMouseX = 0;
    int currentMouseY = 0;

public:
    EventHandler() = default;
    ~EventHandler() = default;

    FrameInput pollEvents();
    void clearInputState() {
        quitRequested = false;
        pressedKeys.clear();
        justPressedKeys.clear();
        justPressedScancodes.clear();
        mouseLeftHeld = false;
    }
    /* No permito copias */
    EventHandler(const EventHandler&) = delete;
    EventHandler& operator=(const EventHandler&) = delete;

    /* Permito movimientos */
    EventHandler(EventHandler&&) = default;
    EventHandler& operator=(EventHandler&&) = default;
};

#endif
