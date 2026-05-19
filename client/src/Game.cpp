#include "Game.h"

#include <SDL2/SDL.h>

namespace {
constexpr int TILE_SIZE = 32;
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;
}  // namespace

Game::Game(Client& client):
        sdl(SDL_INIT_VIDEO),
        window("Argentum Online - Cliente", WINDOW_WIDTH, WINDOW_HEIGHT),
        events(),
        client(client) {}

void Game::run() {
    while (true) {
        const FrameInput input = events.pollEvents();
        if (input.quit) {
            break;
        }
        render();
        SDL_Delay(16);
    }
}

void Game::render() {
    SDL2pp::Renderer& renderer = window.getRenderer();

    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    renderer.SetDrawColor(255, 255, 255, 255);

    SnapshotDTO snap = client.getLatestSnapshot();
    for (const EntityDTO& entity: snap.entities) {
        const SDL2pp::Rect rect(
                entity.x * TILE_SIZE,
                entity.y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE);
        renderer.FillRect(rect);
    }

    renderer.Present();
}
