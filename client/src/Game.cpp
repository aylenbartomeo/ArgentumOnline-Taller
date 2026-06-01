#include "Game.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

#include "common/include/dto/StartMoveDTO.h"

#include "CharacterSprites.h"
#include "HealthBar.h"
#include "OverlayRegistry.h"

namespace {
constexpr int TILE_SIZE = 32;
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;
constexpr Uint32 MOVE_INTERVAL_MS = 200;

constexpr const char* RESOURCES_DIR = "resources/";
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

constexpr const char* HEALTHBAR_SHEET = "en_barradevida.bmp";

constexpr const char* GROUND_SHEET = "5108.png";
constexpr int GROUND_SRC_X = 416;
constexpr int GROUND_SRC_Y = 384;
constexpr int DARK_GROUND_SRC_X = 512;
constexpr int DARK_GROUND_SRC_Y = 480;
constexpr int GROUND_TILE = 32;

const char* citizenSheet(const std::string& type) {
    if (type == "merchant")
        return "1077.png";
    if (type == "banker")
        return "1071.png";
    if (type == "priest")
        return "1910.png";
    return "1200.png";
}

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

    const CameraOffset camera = computeCamera();
    renderTerrain(camera);
    renderOverlays(camera);
    renderGroundItems(camera);
    renderCitizens(camera);
    renderEntities(camera);

    renderer.Present();
}

CameraOffset Game::computeCamera() {
    const uint32_t myId = client.getClientId();
    int focusX = 0;
    int focusY = 0;
    auto it = std::find_if(lastSnapshot.players.begin(), lastSnapshot.players.end(),
                           [myId](const EntityDTO& entity) { return entity.id == myId; });
    if (it != lastSnapshot.players.end()) {
        focusX = it->x * TILE_SIZE + TILE_SIZE / 2;
        focusY = it->y * TILE_SIZE + TILE_SIZE / 2;
    }
    return computeCameraOffset(focusX, focusY, WINDOW_WIDTH, WINDOW_HEIGHT,
                               map.getWidth() * TILE_SIZE, map.getHeight() * TILE_SIZE);
}

void Game::renderTerrain(const CameraOffset& camera) {
    SDL2pp::Renderer& renderer = window.getRenderer();
    SDL2pp::Texture& ground = textures.get(std::string(RESOURCES_DIR) + GROUND_SHEET);
    const SDL2pp::Rect groundSrc(GROUND_SRC_X, GROUND_SRC_Y, GROUND_TILE, GROUND_TILE);
    const SDL2pp::Rect darkGroundSrc(DARK_GROUND_SRC_X, DARK_GROUND_SRC_Y, GROUND_TILE,
                                     GROUND_TILE);

    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            const SDL2pp::Rect dstRect(col * TILE_SIZE - camera.x, row * TILE_SIZE - camera.y,
                                       TILE_SIZE, TILE_SIZE);
            renderer.Copy(ground, cellInSafeZone(col, row) ? darkGroundSrc : groundSrc, dstRect);
        }
    }
    renderer.SetClipRect();
}

void Game::renderGroundItems(const CameraOffset& camera) {
    SDL2pp::Renderer& renderer = window.getRenderer();
    const std::vector<OverlayDef>& registry = getOverlayRegistry();

    for (const auto& item: lastSnapshot.groundItems) {
        auto it = std::find_if(registry.begin(), registry.end(), [&item](const OverlayDef& def) {
            return static_cast<uint32_t>(def.itemId) == item.itemId;
        });

        if (it != registry.end()) {
            const OverlayDef& def = *it;
            SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + def.tilesheet);
            const SDL2pp::Rect srcRect(def.srcX, def.srcY, def.srcW, def.srcH);
            const int dstW = TILE_SIZE;
            const int dstH = (def.srcH * TILE_SIZE) / def.srcW;
            const int dstX = item.x * TILE_SIZE - camera.x;
            const int dstY = item.y * TILE_SIZE + TILE_SIZE - dstH - camera.y;

            renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));
        }
    }
}

void Game::renderCitizens(const CameraOffset& camera) {
    SDL2pp::Renderer& renderer = window.getRenderer();
    const SDL2pp::Rect srcRect(CHARACTER_FRAME_X, CHARACTER_FRAME_Y, CHARACTER_FRAME_W,
                               CHARACTER_FRAME_H);
    for (const MapCitizen& citizen: map.getCitizens()) {
        SDL2pp::Texture& body =
                textures.get(std::string(RESOURCES_DIR) + citizenSheet(citizen.type));
        const SDL2pp::Rect dstRect(citizen.x * TILE_SIZE - camera.x,
                                   citizen.y * TILE_SIZE + TILE_SIZE - CHARACTER_DRAW_H - camera.y,
                                   TILE_SIZE, CHARACTER_DRAW_H);
        renderer.Copy(body, srcRect, dstRect);
    }
}

bool Game::cellInSafeZone(int col, int row) const {
    const auto& zones = map.getSafeZones();
    return std::any_of(zones.begin(), zones.end(), [col, row](const SafeZoneRect& zone) {
        return col >= zone.x && col < zone.x + zone.width && row >= zone.y &&
               row < zone.y + zone.height;
    });
}

void Game::renderOverlays(const CameraOffset& camera) {
    SDL2pp::Renderer& renderer = window.getRenderer();
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int tileId = map.tileAt(col, row);
            if (tileId <= 0 || tileId > static_cast<int>(registry.size())) {
                continue;
            }
            const OverlayDef& def = registry[tileId - 1];
            if (!def.solid) {
                continue;
            }
            SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + def.tilesheet);
            const SDL2pp::Rect srcRect(def.srcX, def.srcY, def.srcW, def.srcH);
            const int dstW = TILE_SIZE;
            const int dstH = (def.srcH * TILE_SIZE) / def.srcW;
            const int dstX = col * TILE_SIZE - camera.x;
            const int dstY = row * TILE_SIZE + TILE_SIZE - dstH - camera.y;
            renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));
        }
    }
}

void Game::renderEntities(const CameraOffset& camera) {
    SDL2pp::Renderer& renderer = window.getRenderer();
    SDL2pp::Texture& headSheet = textures.get(std::string(RESOURCES_DIR) + HEAD_SHEET);
    SDL2pp::Texture& barSheet = textures.get(std::string(RESOURCES_DIR) + HEALTHBAR_SHEET);
    const uint32_t myId = client.getClientId();

    const SDL2pp::Rect srcRect(CHARACTER_FRAME_X, CHARACTER_FRAME_Y, CHARACTER_FRAME_W,
                               CHARACTER_FRAME_H);
    const SDL2pp::Rect headSrc(HEAD_FRAME_X, HEAD_FRAME_Y, HEAD_FRAME_W, HEAD_FRAME_H);

    auto drawEntity = [&](const EntityDTO& entity) {
        const EntitySprite sprite = spriteForEntity(entity.type, entity.sprite_id);
        SDL2pp::Texture& body = textures.get(std::string(RESOURCES_DIR) + sprite.bodySheet);
        const SDL2pp::Rect dstRect(entity.x * TILE_SIZE - camera.x,
                                   entity.y * TILE_SIZE + TILE_SIZE - CHARACTER_DRAW_H - camera.y,
                                   TILE_SIZE, CHARACTER_DRAW_H);
        renderer.Copy(body, srcRect, dstRect);

        if (sprite.drawHead) {
            const int headX = entity.x * TILE_SIZE + TILE_SIZE / 2 - HEAD_DRAW_W / 2 - camera.x;
            const int headY = entity.y * TILE_SIZE + TILE_SIZE - CHARACTER_DRAW_H +
                              sprite.headOverlap - HEAD_DRAW_H - camera.y;
            renderer.Copy(headSheet, headSrc, SDL2pp::Rect(headX, headY, HEAD_DRAW_W, HEAD_DRAW_H));
        }

        if (entity.type == EntityType::PLAYER && entity.id == myId) {
            renderer.SetDrawColor(255, 235, 0, 255);
            const int cx = entity.x * TILE_SIZE + TILE_SIZE / 2 - MARKER_SHIFT_X - camera.x;
            const int cy = entity.y * TILE_SIZE + TILE_SIZE - 4 - camera.y;
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
    };

    for (const EntityDTO& player: lastSnapshot.players) {
        drawEntity(player);
    }
    for (const EntityDTO& monster: lastSnapshot.monsters) {
        drawEntity(monster);
    }

    const SDL2pp::Rect barSrc(0, 0, barSheet.GetWidth(), barSheet.GetHeight());
    auto drawHealthBar = [&](const EntityDTO& entity) {
        const HealthBarLayout bar =
                computeHealthBar(entity.current_hp, entity.max_hp, entity.x * TILE_SIZE - camera.x,
                                 entity.y * TILE_SIZE - camera.y, TILE_SIZE);
        if (!bar.visible) {
            return;
        }
        renderer.SetDrawColor(20, 20, 20, 255);
        renderer.FillRect(SDL2pp::Rect(bar.background.x, bar.background.y, bar.background.w,
                                       bar.background.h));
        if (bar.fill.w > 0) {
            renderer.Copy(barSheet, barSrc,
                          SDL2pp::Rect(bar.fill.x, bar.fill.y, bar.fill.w, bar.fill.h));
        }
    };
    for (const EntityDTO& player: lastSnapshot.players) {
        drawHealthBar(player);
    }
    for (const EntityDTO& monster: lastSnapshot.monsters) {
        drawHealthBar(monster);
    }
}
