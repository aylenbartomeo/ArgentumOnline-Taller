#include "MapRenderer.h"

#include "OverlayRegistry.h"

namespace {
constexpr const char* RESOURCES_DIR = "resources/";
constexpr int TILE_NATIVE = 32;

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
            SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + "ground/" +
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
            SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + "decoration/" +
                                                std::to_string(v - 1) + ".png");
            Position screen = camera.cellToScreen(col, row);
            const int dstW = tex.GetWidth() * ts / TILE_NATIVE;
            const int dstH = tex.GetHeight() * ts / TILE_NATIVE;
            const int dstY = screen.y + ts - dstH;
            if (screen.x + dstW <= 0 || screen.x >= canvasW || dstY + dstH <= 0 || dstY >= canvasH) {
                continue;
            }
            renderer.Copy(tex, SDL2pp::Rect(0, 0, tex.GetWidth(), tex.GetHeight()),
                          SDL2pp::Rect(screen.x, dstY, dstW, dstH));
        }
    }
}

void MapRenderer::drawItems(const EditorMap& map, const Camera& camera, int canvasW, int canvasH) {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    const int ts = camera.getTileSize();
    for (const auto& entry: map.getItems()) {
        int col = entry.first.first;
        int row = entry.first.second;
        const PlacedItem& item = entry.second;
        Position screen = camera.cellToScreen(col, row);
        if (screen.x + ts <= 0 || screen.x >= canvasW || screen.y + ts <= 0 || screen.y >= canvasH) {
            continue;
        }
        drawOverlaySprite(renderer, textures, registry[item.overlayIndex], screen.x, screen.y, ts);
    }
}

void MapRenderer::render(const EditorMap& map, const Camera& camera, int canvasW, int canvasH) {
    drawGroundLayer(map, camera, canvasW, canvasH, map.getGround());
    drawGroundLayer(map, camera, canvasW, canvasH, map.getGround2());
    drawDecorationLayer(map, camera, canvasW, canvasH);
    drawItems(map, camera, canvasW, canvasH);
}
