#include "WorldRenderer.h"

#include <algorithm>
#include <fstream>
#include <optional>
#include <string>

#include "../animation/CharacterSprites.h"
#include "../common/GameConstants.h"
#include "../rendering/BuildingFronts.h"
#include "../rendering/NpcVisuals.h"
#include "../rendering/TallFlora.h"
#include "../rendering/TileDraw.h"
#include "../ui/GroundItemLabel.h"

#include "OverlayRegistry.h"

namespace GC = GameConstants;

namespace {
constexpr int CHARACTER_FRAME_X = 2;
constexpr int CHARACTER_FRAME_Y = 4;
constexpr int CHARACTER_FRAME_W = 24;
constexpr int CHARACTER_FRAME_H = 44;
constexpr int CITIZEN_HEAD_OVERLAP = 6;
}  // namespace

// ─── Constructor ──────────────────────────────────────────────────────────────
WorldRenderer::WorldRenderer(TextureManager& textures, SDL2pp::Renderer& renderer,
                             const TileMap& map):
        textures(textures),
        renderer(renderer),
        map(map),
        worldFont(nullptr),
        indoorRegions(map.getIndoor()) {}

bool WorldRenderer::cellInSafeZone(int col, int row) const {
    return std::any_of(map.getSafeZones().begin(), map.getSafeZones().end(),
                       [col, row](const SafeZoneRect& zone) {
                           return col >= zone.x && col < zone.x + zone.width && row >= zone.y &&
                                  row < zone.y + zone.height;
                       });
}

// ─── Terrain ─────────────────────────────────────────────────────────────────
void WorldRenderer::renderTerrain(const CameraOffset& camera) const {
    renderGroundLayer(map.getGround(), camera);
    renderGroundLayer(map.getGround2(), camera);
    renderer.SetClipRect();
}

void WorldRenderer::renderDecorationBehind(const CameraOffset& camera, int playerRow) const {
    const std::vector<std::vector<int>>& grid = map.getDecoration();
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int v = grid[row][col];
            if (v <= 0) {
                continue;
            }
            if (isTallFlora(v - 1) && playerRow >= 0 && row > playerRow) {
                continue;
            }
            drawDecorationTile(v - 1, col, row, camera);
        }
    }
}

void WorldRenderer::renderDecorationFront(const CameraOffset& camera, int playerRow) const {
    if (playerRow < 0) {
        return;
    }
    const std::vector<std::vector<int>>& grid = map.getDecoration();
    for (int row = playerRow + 1; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int v = grid[row][col];
            if (v <= 0 || !isTallFlora(v - 1)) {
                continue;
            }
            drawDecorationTile(v - 1, col, row, camera);
        }
    }
}

void WorldRenderer::drawDecorationTile(int id, int col, int row, const CameraOffset& camera) const {
    SDL2pp::Texture& tex = textures.get(std::string(GC::RESOURCES_DIR) + "decoration/" +
                                        std::to_string(id) + ".png");
    TileRect r = tileDestRect(col, row, tex.GetWidth(), tex.GetHeight(), GC::TILE_SIZE, camera.x,
                              camera.y);
    renderer.Copy(tex, SDL2pp::Rect(0, 0, tex.GetWidth(), tex.GetHeight()),
                  SDL2pp::Rect(r.x, r.y, r.w, r.h));
}

void WorldRenderer::renderGroundLayer(const std::vector<std::vector<int>>& grid,
                                      const CameraOffset& camera) const {
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int v = grid[row][col];
            if (v <= 0) {
                continue;
            }
            SDL2pp::Texture& tex = textures.get(std::string(GC::RESOURCES_DIR) + "ground/" +
                                                std::to_string(v - 1) + ".png");
            renderer.Copy(
                    tex, SDL2pp::Rect(0, 0, tex.GetWidth(), tex.GetHeight()),
                    SDL2pp::Rect(col * GC::TILE_SIZE - camera.x, row * GC::TILE_SIZE - camera.y,
                                 tex.GetWidth(), tex.GetHeight()));
        }
    }
}

void WorldRenderer::renderRoofs(const CameraOffset& camera, int playerCol, int playerRow) const {
    const std::vector<std::vector<int>>& grid = map.getRoofs();
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int v = grid[row][col];
            if (v <= 0) {
                continue;
            }
            if (indoorRegions.roofHidden(col, row, playerCol, playerRow)) {
                continue;
            }
            SDL2pp::Texture& tex = textures.get(std::string(GC::RESOURCES_DIR) + "decoration/" +
                                                std::to_string(v - 1) + ".png");
            TileRect r = tileDestRect(col, row, tex.GetWidth(), tex.GetHeight(), GC::TILE_SIZE,
                                      camera.x, camera.y);
            r.y += roofDropPixels(v);
            r.x += roofShiftX(v);
            renderer.Copy(tex, SDL2pp::Rect(0, 0, tex.GetWidth(), tex.GetHeight()),
                          SDL2pp::Rect(r.x, r.y, r.w, r.h));
        }
    }
}

void WorldRenderer::renderBuildingFronts(const CameraOffset& camera, int playerCol,
                                         int playerRow) const {
    const std::vector<std::vector<int>>& grid = map.getDecoration();
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int v = grid[row][col];
            std::optional<int> bandTiles = buildingFrontTiles(v);
            if (!bandTiles) {
                continue;
            }
            if (!indoorRegions.sameRegion(col, row, playerCol, playerRow)) {
                continue;
            }
            SDL2pp::Texture& tex = textures.get(std::string(GC::RESOURCES_DIR) + "decoration/" +
                                                std::to_string(v - 1) + ".png");
            TileRect full = tileDestRect(col, row, tex.GetWidth(), tex.GetHeight(), GC::TILE_SIZE,
                                         camera.x, camera.y);
            TileRect band = bottomBandRect(full, *bandTiles, GC::TILE_SIZE);
            renderer.Copy(tex, SDL2pp::Rect(0, tex.GetHeight() - band.h, tex.GetWidth(), band.h),
                          SDL2pp::Rect(band.x, band.y, band.w, band.h));
        }
    }
}

// ─── Overlays ────────────────────────────────────────────────────────────────
void WorldRenderer::renderOverlays(const CameraOffset& camera) const {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int tileId = map.tileAt(col, row);
            if (tileId <= 0 || tileId > static_cast<int>(registry.size()))
                continue;
            const OverlayDef& def = registry[tileId - 1];
            if (!def.solid)
                continue;
            SDL2pp::Texture& tex = textures.get(std::string(GC::RESOURCES_DIR) + def.tilesheet);
            const SDL2pp::Rect srcRect(def.srcX, def.srcY, def.srcW, def.srcH);
            const int dstW = GC::TILE_SIZE;
            const int dstH = (def.srcH * GC::TILE_SIZE) / def.srcW;
            const int dstX = col * GC::TILE_SIZE - camera.x;
            const int dstY = row * GC::TILE_SIZE + GC::TILE_SIZE - dstH - camera.y;
            renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));
        }
    }
}

// ─── Ground items ─────────────────────────────────────────────────────────────
void WorldRenderer::renderGroundItems(const CameraOffset& camera,
                                      const SnapshotDTO& snapshot) const {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();

    for (const auto& item: snapshot.groundItems) {
        auto it = std::find_if(registry.begin(), registry.end(), [&item](const OverlayDef& def) {
            return static_cast<uint32_t>(def.itemId) == item.itemId;
        });
        if (it == registry.end())
            continue;

        const OverlayDef& def = *it;
        SDL2pp::Texture& tex = textures.get(std::string(GC::RESOURCES_DIR) + def.tilesheet);
        const SDL2pp::Rect srcRect(def.srcX, def.srcY, def.srcW, def.srcH);
        int dstW = def.srcW;
        int dstH = def.srcH;
        if (dstW > GC::TILE_SIZE) {
            dstH = def.srcH * GC::TILE_SIZE / def.srcW;
            dstW = GC::TILE_SIZE;
        }
        const int dstX = item.x * GC::TILE_SIZE + (GC::TILE_SIZE - dstW) / 2 - camera.x;
        const int dstY = item.y * GC::TILE_SIZE + GC::TILE_SIZE - dstH - camera.y;
        renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));

        if (auto label = groundAmountLabel(item.amount))
            drawGroundAmount(*label, item.x, item.y, camera);
    }
}

void WorldRenderer::drawGroundAmount(const std::string& text, int tileX, int tileY,
                                     const CameraOffset& camera) const {
    if (!worldFont)
        return;
    const SDL_Color white{255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderUTF8_Blended(worldFont, text.c_str(), white);
    if (!surf)
        return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer.Get(), surf);
    const int w = surf->w;
    const int h = surf->h;
    SDL_FreeSurface(surf);
    if (!tex)
        return;
    const int x = tileX * GC::TILE_SIZE + GC::TILE_SIZE / 2 - w / 2 - camera.x;
    const int y = tileY * GC::TILE_SIZE + GC::TILE_SIZE - h - camera.y;
    SDL_SetTextureColorMod(tex, 0, 0, 0);
    SDL_Rect shadow{x + 1, y + 1, w, h};
    SDL_RenderCopy(renderer.Get(), tex, nullptr, &shadow);
    SDL_SetTextureColorMod(tex, 255, 255, 255);
    SDL_Rect dst{x, y, w, h};
    SDL_RenderCopy(renderer.Get(), tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

// ─── Citizens ────────────────────────────────────────────────────────────────
void WorldRenderer::renderCitizens(const CameraOffset& camera,
                                   std::optional<uint32_t> selectedNpc) const {
    SDL2pp::Texture& headSheet = textures.get(std::string(GC::RESOURCES_DIR) + GC::HEAD_SHEET);
    const SDL2pp::Rect srcRect(CHARACTER_FRAME_X, CHARACTER_FRAME_Y, CHARACTER_FRAME_W,
                               CHARACTER_FRAME_H);

    for (const MapCitizen& citizen: map.getCitizens()) {
        const int tilePixelX = citizen.x * GC::TILE_SIZE;
        const int tilePixelY = citizen.y * GC::TILE_SIZE;

        // Cuerpo
        SDL2pp::Texture& body =
                textures.get(std::string(GC::RESOURCES_DIR) + NpcVisuals::bodySheet(citizen.type));
        renderer.Copy(body, srcRect,
                      SDL2pp::Rect(tilePixelX - camera.x,
                                   tilePixelY + GC::TILE_SIZE - GC::CHARACTER_DRAW_H - camera.y,
                                   GC::TILE_SIZE, GC::CHARACTER_DRAW_H));

        // Cabeza
        const FrameRect hf = NpcVisuals::headRect(citizen.type);
        const int headX = tilePixelX + GC::TILE_SIZE / 2 - GC::HEAD_DRAW_W / 2 - camera.x;
        const int headY = tilePixelY + GC::TILE_SIZE - GC::CHARACTER_DRAW_H + CITIZEN_HEAD_OVERLAP -
                          GC::HEAD_DRAW_H - camera.y;
        renderer.Copy(headSheet, SDL2pp::Rect(hf.x, hf.y, hf.w, hf.h),
                      SDL2pp::Rect(headX, headY, GC::HEAD_DRAW_W, GC::HEAD_DRAW_H));

        // Anillo de selección (verde)
        if (selectedNpc && NpcVisuals::matchesFakeId(*selectedNpc, citizen.x, citizen.y)) {
            NpcVisuals::drawSelectionEllipse(renderer, tilePixelX - camera.x, tilePixelY - camera.y,
                                             GC::TILE_SIZE, 0, 255, 0);
        }
    }
}
