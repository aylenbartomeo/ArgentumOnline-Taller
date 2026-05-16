#include "Game.h"

#include <SDL2/SDL.h>
#include <cmath>

namespace {
constexpr float PLAYER_SPEED = 200.0f;
constexpr int   PLAYER_SIZE = 32;
constexpr int   WINDOW_WIDTH = 640;
constexpr int   WINDOW_HEIGHT = 480;
}  // namespace

Game::Game()
    : sdl(SDL_INIT_VIDEO)
    , window("Argentum Online - Cliente", WINDOW_WIDTH, WINDOW_HEIGHT)
    , events()
    , playerX((WINDOW_WIDTH - PLAYER_SIZE) / 2.0f)
    , playerY((WINDOW_HEIGHT - PLAYER_SIZE) / 2.0f) {}

void Game::run() {
    Uint64 prev = SDL_GetPerformanceCounter();
    const float freq = static_cast<float>(SDL_GetPerformanceFrequency());

    while (true) {
        const Uint64 now = SDL_GetPerformanceCounter();
        const float dt = (now - prev) / freq;
        prev = now;

        const FrameInput input = events.pollEvents();
        if (input.quit) {
            break;
        }
        update(input, dt);
        render();

        SDL_Delay(16);
    }
}

void Game::update(const FrameInput& input, float dt) {
    float dx = 0.0f;
    float dy = 0.0f;
    if (input.moveNorth) dy -= 1.0f;
    if (input.moveSouth) dy += 1.0f;
    if (input.moveEast)  dx += 1.0f;
    if (input.moveWest)  dx -= 1.0f;

    if (dx != 0.0f && dy != 0.0f) {
        const float invSqrt2 = 1.0f / std::sqrt(2.0f);
        dx *= invSqrt2;
        dy *= invSqrt2;
    }

    playerX += dx * PLAYER_SPEED * dt;
    playerY += dy * PLAYER_SPEED * dt;

    const int currentWidth = window.getWidth();
    const int currentHeight = window.getHeight();

    if (playerX < 0.0f) {
        playerX = 0.0f;
    }
    if (playerX > currentWidth - PLAYER_SIZE) {
        playerX = static_cast<float>(currentWidth - PLAYER_SIZE);
    }
    if (playerY < 0.0f) {
        playerY = 0.0f;
    }
    if (playerY > currentHeight - PLAYER_SIZE) {
        playerY = static_cast<float>(currentHeight - PLAYER_SIZE);
    }
}

void Game::render() {
    SDL2pp::Renderer& renderer = window.getRenderer();

    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    renderer.SetDrawColor(255, 255, 255, 255);
    const SDL2pp::Rect rect(
        static_cast<int>(playerX),
        static_cast<int>(playerY),
        PLAYER_SIZE,
        PLAYER_SIZE);
    renderer.FillRect(rect);

    renderer.Present();
}
