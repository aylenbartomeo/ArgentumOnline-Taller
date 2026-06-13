#ifndef GAME_H
#define GAME_H

#include <SDL2pp/SDL2pp.hh>
#include <SDL_ttf.h>

#include "../animation/CharacterAnimator.h"
#include "../input/ChatCommandParser.h"
#include "../input/EventHandler.h"
#include "../rendering/TextureManager.h"
#include "../rendering/TileMap.h"
#include "../rendering/Viewport.h"
#include "../ui/HudPanel.h"
#include "../ui/ManualPanel.h"
#include "../ui/MiniChat.h"
#include "../ui/Window.h"
#include "common/include/dto/PlayerStatsDTO.h"
#include "common/include/dto/Snapshot.h"
#include "systems/AudioSystem.h"
#include "systems/CameraSystem.h"
#include "systems/EntityRenderer.h"
#include "systems/FxSystem.h"
#include "systems/InputProcessor.h"
#include "systems/WorldRenderer.h"

#include "Client.h"

class Game {
public:
    explicit Game(Client& client);
    ~Game();

    void run();
    bool runStartupAndCreation();

    SDL2pp::Renderer& getWindowRenderer() { return window.getRenderer(); }
    TextureManager& getTextures() { return textures; }

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;

private:
    void render(const FrameInput& input);

    // SDL / core
    SDL2pp::SDL sdl;
    Window window;
    EventHandler events;
    Client& client;

    // Resources
    TextureManager textures;
    TileMap map;
    TTF_Font* worldFont = nullptr;
    TTF_Font* entityFont = nullptr;

    // UI
    MiniChat miniChat;
    HudPanel hud;
    ManualPanel manualPanel;
    ChatCommandParser chatParser;

    // State
    SnapshotDTO lastSnapshot;
    PlayerStatsDTO lastStats;

    // Systems (order matches render pipeline)
    AudioSystem audio;
    CameraSystem camera;
    WorldRenderer worldRenderer;
    EntityRenderer entityRenderer;
    FxSystem fxSystem;
    InputProcessor inputProcessor;
};

#endif
