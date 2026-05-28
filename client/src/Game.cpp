#include "Game.h"

#include <cmath>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include <SDL2/SDL.h>

#include "common/include/dto/StartMoveDTO.h"

namespace {
constexpr int TILE_SIZE = 32;
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;
constexpr Uint32 MOVE_INTERVAL_MS = 200;

constexpr const char* RESOURCES_DIR = "resources/";
constexpr const char* CHARACTER_SHEET = "1500.png";
constexpr int CHARACTER_FRAME_X = 2;
constexpr int CHARACTER_FRAME_Y = 4;
constexpr int CHARACTER_FRAME_W = 24;
constexpr int CHARACTER_FRAME_H = 44;
constexpr int CHARACTER_DRAW_H = TILE_SIZE * 3 / 2;
constexpr double TAU = 6.283185307179586;
constexpr int MARKER_SEGMENTS = 24;
constexpr int MARKER_SHIFT_X = 3;

constexpr const char* HEAD_SHEET = "420.png";
constexpr int HEAD_FRAME_X = 6;
constexpr int HEAD_FRAME_Y = 13;
constexpr int HEAD_FRAME_W = 13;
constexpr int HEAD_FRAME_H = 15;
constexpr int HEAD_DRAW_W = 18;
constexpr int HEAD_DRAW_H = 20;
constexpr int HEAD_OVERLAP = 6;

std::string readWholeFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("No pude abrir el archivo: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
}  // namespace

Game::Game(Client& client):
        sdl(SDL_INIT_VIDEO),
        window("Argentum Online - Client", WINDOW_WIDTH, WINDOW_HEIGHT),
        events(),
        client(client),
        textures(window.getRenderer()),
        map(readWholeFile("maps/defaultMap.json")),
        lastSnapshot(),
        lastMoveSentMs(0) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    window.getRenderer().SetLogicalSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

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
    if (input.moveNorth)
        direction = Movement::UP;
    else if (input.moveSouth)
        direction = Movement::DOWN;
    else if (input.moveEast)
        direction = Movement::RIGHT;
    else if (input.moveWest)
        direction = Movement::LEFT;

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

    renderTerrain();
    renderEntities();

    renderer.Present();
}

void Game::renderTerrain() {
    SDL2pp::Renderer& renderer = window.getRenderer();
    SDL2pp::Texture& tileset = textures.get(std::string(RESOURCES_DIR) + map.getTileset());
    const int src = map.getTileSize();
    const int cols = map.getTilesetCols();

    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            const int id = map.tileAt(col, row);
            const SDL2pp::Rect srcRect((id % cols) * src, (id / cols) * src, src, src);
            const SDL2pp::Rect dstRect(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            renderer.Copy(tileset, srcRect, dstRect);
        }
    }
}

void Game::renderEntities() {
    SDL2pp::Renderer& renderer = window.getRenderer();
    SDL2pp::Texture& sheet = textures.get(std::string(RESOURCES_DIR) + CHARACTER_SHEET);
    SDL2pp::Texture& headSheet = textures.get(std::string(RESOURCES_DIR) + HEAD_SHEET);
    const uint32_t myId = client.getClientId();

    const SDL2pp::Rect srcRect(CHARACTER_FRAME_X, CHARACTER_FRAME_Y, CHARACTER_FRAME_W,
                               CHARACTER_FRAME_H);
    const SDL2pp::Rect headSrc(HEAD_FRAME_X, HEAD_FRAME_Y, HEAD_FRAME_W, HEAD_FRAME_H);

    for (const EntityDTO& entity: lastSnapshot.entities) {
        const SDL2pp::Rect dstRect(entity.x * TILE_SIZE,
                                   entity.y * TILE_SIZE + TILE_SIZE - CHARACTER_DRAW_H, TILE_SIZE,
                                   CHARACTER_DRAW_H);
        renderer.Copy(sheet, srcRect, dstRect);

        const int headX = entity.x * TILE_SIZE + TILE_SIZE / 2 - HEAD_DRAW_W / 2;
        const int headY = entity.y * TILE_SIZE + TILE_SIZE - CHARACTER_DRAW_H + HEAD_OVERLAP -
                          HEAD_DRAW_H;
        renderer.Copy(headSheet, headSrc, SDL2pp::Rect(headX, headY, HEAD_DRAW_W, HEAD_DRAW_H));

        if (entity.id == myId) {
            renderer.SetDrawColor(255, 235, 0, 255);
            const int cx = entity.x * TILE_SIZE + TILE_SIZE / 2 - MARKER_SHIFT_X;
            const int cy = entity.y * TILE_SIZE + TILE_SIZE - 4;
            for (int t = -1; t <= 1; ++t) {
                const int rx = TILE_SIZE / 2 - 2 + t;
                const int ry = TILE_SIZE / 5 + t;
                for (int i = 0; i < MARKER_SEGMENTS; ++i) {
                    const double a0 = TAU * i / MARKER_SEGMENTS;
                    const double a1 = TAU * (i + 1) / MARKER_SEGMENTS;
                    renderer.DrawLine(cx + static_cast<int>(rx * std::cos(a0)),
                                      cy + static_cast<int>(ry * std::sin(a0)),
                                      cx + static_cast<int>(rx * std::cos(a1)),
                                      cy + static_cast<int>(ry * std::sin(a1)));
                }
            }
        }
    }
}
