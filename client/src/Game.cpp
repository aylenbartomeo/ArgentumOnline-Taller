#include "Game.h"

#include <optional>

#include <SDL2/SDL.h>

#include "common/include/dto/StartMoveDTO.h"

namespace {
constexpr int TILE_SIZE = 32;
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;
constexpr Uint32 MOVE_INTERVAL_MS = 200;
}  // namespace

Game::Game(Client& client):
        sdl(SDL_INIT_VIDEO),
        window("Argentum Online - Client", WINDOW_WIDTH, WINDOW_HEIGHT),
        events(),
        client(client),
        lastSnapshot(),
        lastMoveSentMs(0) {}

void Game::run() {
    while (true) {
        const FrameInput input = events.pollEvents();
        if (input.quit) {
            break;
        }
        sendMoveIfDue(input);
        render();
        SDL_Delay(16);
    }
}

void Game::sendMoveIfDue(const FrameInput& input) {
    const Uint32 now = SDL_GetTicks();
    if (now - lastMoveSentMs < MOVE_INTERVAL_MS) {
        return;
    }

    std::optional<Movement> direction;
    if (input.moveNorth) direction = Movement::UP;
    else if (input.moveSouth) direction = Movement::DOWN;
    else if (input.moveEast) direction = Movement::RIGHT;
    else if (input.moveWest) direction = Movement::LEFT;

    if (direction) {
        client.sendCommand(StartMoveDTO(*direction));
        lastMoveSentMs = now;
    }
}

void Game::render() {
    SnapshotDTO incoming;
    while (client.tryPopSnapshot(incoming)) {
        lastSnapshot = incoming;
    }

    SDL2pp::Renderer& renderer = window.getRenderer();

    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    const uint32_t myId = client.getClientId();
    for (const EntityDTO& entity: lastSnapshot.entities) {
        if (entity.id == myId) {
            renderer.SetDrawColor(0, 255, 0, 255);
        } else {
            renderer.SetDrawColor(255, 255, 255, 255);
        }
        const SDL2pp::Rect rect(
                entity.x * TILE_SIZE,
                entity.y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE);
        renderer.FillRect(rect);
    }

    renderer.Present();
}
