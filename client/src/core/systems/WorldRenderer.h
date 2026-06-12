#ifndef WORLD_RENDERER_H
#define WORLD_RENDERER_H

#include <optional>
#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>
#include <SDL_ttf.h>

#include "../rendering/IndoorRegions.h"
#include "../rendering/TextureManager.h"
#include "../rendering/TileMap.h"
#include "../rendering/Viewport.h"
#include "common/include/dto/Snapshot.h"

class WorldRenderer {
public:
    WorldRenderer(TextureManager& textures, SDL2pp::Renderer& renderer, const TileMap& map,
                  TTF_Font* worldFont);

    void renderTerrain(const CameraOffset& camera) const;
    void renderDecorationBehind(const CameraOffset& camera, int playerRow) const;
    void renderDecorationFront(const CameraOffset& camera, int playerRow) const;
    void renderOverlays(const CameraOffset& camera) const;
    void renderGroundItems(const CameraOffset& camera, const SnapshotDTO& snapshot) const;
    void renderRoofs(const CameraOffset& camera, int playerCol, int playerRow) const;
    void renderCitizens(const CameraOffset& camera,
                        std::optional<uint32_t> selectedNpc = std::nullopt) const;

private:
    void renderGroundLayer(const std::vector<std::vector<int>>& grid,
                           const CameraOffset& camera) const;
    void drawDecorationTile(int id, int col, int row, const CameraOffset& camera) const;
    void drawGroundAmount(const std::string& text, int tileX, int tileY,
                          const CameraOffset& camera) const;

    TextureManager& textures;
    SDL2pp::Renderer& renderer;
    const TileMap& map;
    TTF_Font* worldFont;
    IndoorRegions indoorRegions;
};

#endif
