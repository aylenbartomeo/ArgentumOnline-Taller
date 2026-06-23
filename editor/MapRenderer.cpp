#include "MapRenderer.h"

#include <cmath>
#include <string>
#include <vector>

#include "../client/src/ui/GroundItemLabel.h"

#include "CatalogSprites.h"
#include "OverlayRegistry.h"
#include "SpawnCatalogs.h"

namespace {
constexpr const char* RESOURCES_DIR = "resources/";
constexpr int TILE_NATIVE = 32;
constexpr int HEAD_DRAW_W = 18;
constexpr int HEAD_DRAW_H = 20;
constexpr const char* CHARACTER_SHEET_PATH = "resources/characters/player-body.png";
constexpr int CHARACTER_FRAME_X = 2;
constexpr int CHARACTER_FRAME_Y = 4;
constexpr int CHARACTER_FRAME_W = 24;
constexpr int CHARACTER_FRAME_H = 44;
constexpr const char* HEAD_SHEET_PATH = "resources/characters/heads.png";
constexpr int HEAD_FRAME_X = 6;
constexpr int HEAD_FRAME_Y = 13;
constexpr int HEAD_FRAME_W = 13;
constexpr int HEAD_FRAME_H = 15;
constexpr int HEAD_OVERLAP = 6;
constexpr double TAU = 6.283185307179586;
constexpr int MARKER_SEGMENTS = 24;
constexpr int MARKER_SHIFT_X = 3;

void drawCharacterSprite(SDL2pp::Renderer& renderer, TextureManager& textures, int dstX, int dstY,
                         int dstW, int dstH) {
    SDL2pp::Texture& tex = textures.get(CHARACTER_SHEET_PATH);
    SDL2pp::Rect srcRect(CHARACTER_FRAME_X, CHARACTER_FRAME_Y, CHARACTER_FRAME_W,
                         CHARACTER_FRAME_H);
    renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));
}
}  // namespace

MapRenderer::MapRenderer(SDL2pp::Renderer& renderer, TextureManager& textures):
        renderer(renderer), textures(textures) {}

void MapRenderer::drawGroundLayer(const EditorMap& map, const Camera& camera, int canvasW,
                                  int canvasH, const std::vector<std::vector<int>>& grid) {
    const int ts = camera.getTileSize();
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int v = grid[row][col];
            if (v <= 0) {
                continue;
            }
            SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + "world/ground/" +
                                                std::to_string(v - 1) + ".png");
            Position screen = camera.cellToScreen(col, row);
            const int dstW = tex.GetWidth() * ts / TILE_NATIVE;
            const int dstH = tex.GetHeight() * ts / TILE_NATIVE;
            if (screen.x + dstW <= 0 || screen.x >= canvasW || screen.y + dstH <= 0 ||
                screen.y >= canvasH) {
                continue;
            }
            renderer.Copy(tex, SDL2pp::Rect(0, 0, tex.GetWidth(), tex.GetHeight()),
                          SDL2pp::Rect(screen.x, screen.y, dstW, dstH));
        }
    }
}

void MapRenderer::drawDecorationLayer(const EditorMap& map, const Camera& camera, int canvasW,
                                      int canvasH) {
    const int ts = camera.getTileSize();
    const std::vector<std::vector<int>>& grid = map.getDecoration();
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int v = grid[row][col];
            if (v <= 0) {
                continue;
            }
            SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + "world/decoration/" +
                                                std::to_string(v - 1) + ".png");
            Position screen = camera.cellToScreen(col, row);
            const int dstW = tex.GetWidth() * ts / TILE_NATIVE;
            const int dstH = tex.GetHeight() * ts / TILE_NATIVE;
            const int dstY = screen.y + ts - dstH;
            if (screen.x + dstW <= 0 || screen.x >= canvasW || dstY + dstH <= 0 ||
                dstY >= canvasH) {
                continue;
            }
            renderer.Copy(tex, SDL2pp::Rect(0, 0, tex.GetWidth(), tex.GetHeight()),
                          SDL2pp::Rect(screen.x, dstY, dstW, dstH));
        }
    }
}

void MapRenderer::drawItems(const EditorMap& map, const Camera& camera, int canvasW, int canvasH,
                            Font& labelFont) {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    const int ts = camera.getTileSize();
    for (const auto& entry: map.getItems()) {
        int col = entry.first.first;
        int row = entry.first.second;
        const PlacedItem& item = entry.second;
        Position screen = camera.cellToScreen(col, row);
        if (screen.x + ts <= 0 || screen.x >= canvasW || screen.y + ts <= 0 ||
            screen.y >= canvasH) {
            continue;
        }
        drawOverlaySprite(renderer, textures, registry[item.overlayIndex], screen.x, screen.y, ts);
        if (auto label = groundAmountLabel(static_cast<uint16_t>(item.amount))) {
            const SDL_Color shadow{0, 0, 0, 255};
            const SDL_Color white{255, 255, 255, 255};
            labelFont.drawString(*label, screen.x + 3, screen.y + 3, shadow);
            labelFont.drawString(*label, screen.x + 2, screen.y + 2, white);
        }
    }
}

void MapRenderer::drawMonsters(const EditorMap& map, const Camera& camera, int canvasW,
                               int canvasH) {
    const int ts = camera.getTileSize();
    for (const MonsterSpawn& spawn: map.getMonsters()) {
        const MonsterCatalogEntry* entry = monsterEntryFor(spawn.type);
        if (entry == nullptr) {
            continue;
        }
        Position screen = camera.cellToScreen(spawn.x, spawn.y);
        if (screen.x + ts <= 0 || screen.x >= canvasW || screen.y + ts <= 0 ||
            screen.y >= canvasH) {
            continue;
        }
        drawMonsterSprite(renderer, textures, *entry, screen.x, screen.y, ts);
    }
}

void MapRenderer::drawCitizens(const EditorMap& map, const Camera& camera, int canvasW,
                               int canvasH) {
    const int ts = camera.getTileSize();
    for (const CitizenSpawn& spawn: map.getCitizens()) {
        const CitizenCatalogEntry* entry = citizenEntryFor(spawn.type);
        if (entry == nullptr) {
            continue;
        }
        Position screen = camera.cellToScreen(spawn.x, spawn.y);
        if (screen.x + ts <= 0 || screen.x >= canvasW || screen.y + ts <= 0 ||
            screen.y >= canvasH) {
            continue;
        }
        drawCitizenSprite(renderer, textures, *entry, screen.x, screen.y, ts);
    }
}

void MapRenderer::drawSpawn(const EditorMap& map, const Camera& camera) {
    const int ts = camera.getTileSize();
    Position spawn = map.getSpawn();
    Position screen = camera.cellToScreen(spawn.x, spawn.y);
    const int charH = ts * 3 / 2;
    drawCharacterSprite(renderer, textures, screen.x, screen.y + ts - charH, ts, charH);

    SDL2pp::Texture& headTex = textures.get(HEAD_SHEET_PATH);
    SDL2pp::Rect headSrc(HEAD_FRAME_X, HEAD_FRAME_Y, HEAD_FRAME_W, HEAD_FRAME_H);
    const int headW = HEAD_DRAW_W * ts / TILE_NATIVE;
    const int headH = HEAD_DRAW_H * ts / TILE_NATIVE;
    const int headOverlap = HEAD_OVERLAP * ts / TILE_NATIVE;
    const int headX = screen.x + ts / 2 - headW / 2;
    const int headY = screen.y + ts - charH + headOverlap - headH;
    renderer.Copy(headTex, headSrc, SDL2pp::Rect(headX, headY, headW, headH));

    renderer.SetDrawColor(255, 235, 0, 255);
    const int cx = screen.x + ts / 2 - MARKER_SHIFT_X;
    const int cy = screen.y + ts - 4;
    for (int t = -1; t <= 1; ++t) {
        const int rx = ts / 2 - 2 + t;
        const int ry = ts / 5 + t;
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

void MapRenderer::render(const EditorMap& map, const Camera& camera, int canvasW, int canvasH,
                         Font& labelFont) {
    drawGroundLayer(map, camera, canvasW, canvasH, map.getGround());
    drawGroundLayer(map, camera, canvasW, canvasH, map.getGround2());
    drawDecorationLayer(map, camera, canvasW, canvasH);
    drawItems(map, camera, canvasW, canvasH, labelFont);
    drawMonsters(map, camera, canvasW, canvasH);
    drawCitizens(map, camera, canvasW, canvasH);
    drawSpawn(map, camera);
}
