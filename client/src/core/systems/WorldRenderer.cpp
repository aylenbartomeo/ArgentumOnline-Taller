#include "WorldRenderer.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "../animation/CharacterSprites.h"
#include "../common/GameConstants.h"
#include "../ui/GroundItemLabel.h"

#include "OverlayRegistry.h"

namespace GC = GameConstants;

namespace {
constexpr const char* GROUND_SHEET = "5108.png";
constexpr int GROUND_SRC_X = 416;
constexpr int GROUND_SRC_Y = 384;
constexpr int DARK_GROUND_SRC_X = 512;
constexpr int DARK_GROUND_SRC_Y = 480;
constexpr int GROUND_TILE = 32;

constexpr int CHARACTER_FRAME_X = 2;
constexpr int CHARACTER_FRAME_Y = 4;
constexpr int CHARACTER_FRAME_W = 24;
constexpr int CHARACTER_FRAME_H = 44;
constexpr int CITIZEN_HEAD_OVERLAP = 6;

const char* citizenSheet(const std::string& type) {
    if (type == "merchant")
        return "1077.png";
    if (type == "banker")
        return "1071.png";
    if (type == "priest")
        return "1910.png";
    return "1200.png";
}

FrameRect citizenHead(const std::string& type) {
    if (type == "merchant")
        return FrameRect{115, 13, 13, 15};
    if (type == "banker")
        return FrameRect{142, 13, 13, 15};
    if (type == "priest")
        return FrameRect{170, 13, 11, 15};
    return FrameRect{6, 13, 13, 15};
}
}  // namespace

WorldRenderer::WorldRenderer(TextureManager& textures, SDL2pp::Renderer& renderer,
                             const TileMap& map):
        textures(textures), renderer(renderer), map(map), worldFont(nullptr) {}

bool WorldRenderer::cellInSafeZone(int col, int row) const {
    return std::any_of(map.getSafeZones().begin(), map.getSafeZones().end(),
                       [col, row](const SafeZoneRect& zone) {
                           return col >= zone.x && col < zone.x + zone.width && row >= zone.y &&
                                  row < zone.y + zone.height;
                       });
}

void WorldRenderer::renderTerrain(const CameraOffset& camera) const {
    SDL2pp::Texture& ground = textures.get(std::string(GC::RESOURCES_DIR) + GROUND_SHEET);
    const SDL2pp::Rect groundSrc(GROUND_SRC_X, GROUND_SRC_Y, GROUND_TILE, GROUND_TILE);
    const SDL2pp::Rect darkGroundSrc(DARK_GROUND_SRC_X, DARK_GROUND_SRC_Y, GROUND_TILE,
                                     GROUND_TILE);

    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            const SDL2pp::Rect dst(col * GC::TILE_SIZE - camera.x, row * GC::TILE_SIZE - camera.y,
                                   GC::TILE_SIZE, GC::TILE_SIZE);
            renderer.Copy(ground, cellInSafeZone(col, row) ? darkGroundSrc : groundSrc, dst);
        }
    }
    renderer.SetClipRect();
}

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

        if (auto label = groundAmountLabel(item.amount)) {
            drawGroundAmount(*label, item.x, item.y, camera);
        }
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

void WorldRenderer::renderCitizens(const CameraOffset& camera) const {
    SDL2pp::Texture& headSheet = textures.get(std::string(GC::RESOURCES_DIR) + GC::HEAD_SHEET);
    const SDL2pp::Rect srcRect(CHARACTER_FRAME_X, CHARACTER_FRAME_Y, CHARACTER_FRAME_W,
                               CHARACTER_FRAME_H);
    for (const MapCitizen& citizen: map.getCitizens()) {
        SDL2pp::Texture& body =
                textures.get(std::string(GC::RESOURCES_DIR) + citizenSheet(citizen.type));
        const SDL2pp::Rect dstRect(
                citizen.x * GC::TILE_SIZE - camera.x,
                citizen.y * GC::TILE_SIZE + GC::TILE_SIZE - GC::CHARACTER_DRAW_H - camera.y,
                GC::TILE_SIZE, GC::CHARACTER_DRAW_H);
        renderer.Copy(body, srcRect, dstRect);

        const FrameRect hf = citizenHead(citizen.type);
        const int headX =
                citizen.x * GC::TILE_SIZE + GC::TILE_SIZE / 2 - GC::HEAD_DRAW_W / 2 - camera.x;
        const int headY = citizen.y * GC::TILE_SIZE + GC::TILE_SIZE - GC::CHARACTER_DRAW_H +
                          CITIZEN_HEAD_OVERLAP - GC::HEAD_DRAW_H - camera.y;
        renderer.Copy(headSheet, SDL2pp::Rect(hf.x, hf.y, hf.w, hf.h),
                      SDL2pp::Rect(headX, headY, GC::HEAD_DRAW_W, GC::HEAD_DRAW_H));
    }
}
