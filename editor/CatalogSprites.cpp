#include "CatalogSprites.h"

#include <string>

namespace {
constexpr const char* RESOURCES_DIR = "resources/";
constexpr int TILE_NATIVE = 32;
constexpr int HEAD_DRAW_W = 18;
constexpr int HEAD_DRAW_H = 20;
}  // namespace

void drawOverlaySprite(SDL2pp::Renderer& renderer, TextureManager& textures, const OverlayDef& def,
                       int cellX, int cellY, int cellSize) {
    SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + def.tilesheet);
    const SDL2pp::Rect srcRect(def.srcX, def.srcY, def.srcW, def.srcH);
    int dstW = def.srcW;
    int dstH = def.srcH;
    if (dstW > cellSize) {
        dstH = def.srcH * cellSize / def.srcW;
        dstW = cellSize;
    }
    const int dstX = cellX + (cellSize - dstW) / 2;
    const int dstY = cellY + cellSize - dstH;
    renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));
}

void drawMonsterSprite(SDL2pp::Renderer& renderer, TextureManager& textures,
                       const MonsterCatalogEntry& entry, int cellX, int cellY, int cellSize) {
    SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + entry.sheet);
    const SDL2pp::Rect srcRect(entry.srcX, entry.srcY, entry.srcW, entry.srcH);
    const int box = cellSize * 2;
    int dstW = box;
    int dstH = entry.srcH * box / entry.srcW;
    if (dstH > box) {
        dstH = box;
        dstW = entry.srcW * box / entry.srcH;
    }
    const int dstX = cellX + (cellSize - dstW) / 2;
    const int dstY = cellY + cellSize - dstH;
    renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));
    if (entry.drawHead) {
        SDL2pp::Texture& headTex = textures.get(std::string(RESOURCES_DIR) + entry.headSheet);
        const int headW = HEAD_DRAW_W * cellSize / TILE_NATIVE;
        const int headH = HEAD_DRAW_H * cellSize / TILE_NATIVE;
        const int headX = cellX + (cellSize - headW) / 2;
        const int headY = dstY + entry.headOverlap * cellSize / TILE_NATIVE - headH;
        renderer.Copy(headTex, SDL2pp::Rect(entry.headX, entry.headY, entry.headW, entry.headH),
                      SDL2pp::Rect(headX, headY, headW, headH));
    }
}

void drawCitizenSprite(SDL2pp::Renderer& renderer, TextureManager& textures,
                       const CitizenCatalogEntry& entry, int cellX, int cellY, int cellSize) {
    SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + entry.sheet);
    const SDL2pp::Rect srcRect(entry.srcX, entry.srcY, entry.srcW, entry.srcH);
    const int dstW = cellSize;
    const int dstH = (entry.srcH * cellSize) / entry.srcW;
    const int dstX = cellX;
    const int dstY = cellY + cellSize - dstH;
    renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));
    SDL2pp::Texture& headTex = textures.get(std::string(RESOURCES_DIR) + entry.headSheet);
    const int headW = HEAD_DRAW_W * cellSize / TILE_NATIVE;
    const int headH = HEAD_DRAW_H * cellSize / TILE_NATIVE;
    const int headX = cellX + (cellSize - headW) / 2;
    const int headY = dstY + entry.headOverlap * cellSize / TILE_NATIVE - headH;
    renderer.Copy(headTex, SDL2pp::Rect(entry.headX, entry.headY, entry.headW, entry.headH),
                  SDL2pp::Rect(headX, headY, headW, headH));
}

std::vector<int> itemOverlayIndices() {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    std::vector<int> indices;
    for (size_t i = 0; i < registry.size(); ++i) {
        if (registry[i].itemId != 0) {
            indices.push_back(static_cast<int>(i));
        }
    }
    return indices;
}
