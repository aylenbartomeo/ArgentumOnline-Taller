#ifndef GAME_H
#define GAME_H

#include <SDL2pp/SDL2pp.hh>

#include "common/include/dto/Snapshot.h"

#include "Client.h"
#include "EventHandler.h"
#include "TextureManager.h"
#include "TileMap.h"
#include "Viewport.h"
#include "Window.h"

class Game {
private:
    SDL2pp::SDL sdl;
    Window window;
    EventHandler events;
    Client& client;
    TextureManager textures;
    TileMap map;
    SnapshotDTO lastSnapshot;
    Uint32 lastMoveSentMs;

public:
    explicit Game(Client& client);
    ~Game() = default;

    void run();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    Game(Game&&) = default;
    Game& operator=(Game&&) = default;

private:
    void render();
    void renderTerrain(const CameraOffset& camera);
    void renderOverlays(const CameraOffset& camera);
    void renderCitizens(const CameraOffset& camera);
    bool cellInSafeZone(int col, int row) const;
    void renderEntities(const CameraOffset& camera);
    CameraOffset computeCamera();
    void sendMoveIfDue(const FrameInput& input);
};

#endif
