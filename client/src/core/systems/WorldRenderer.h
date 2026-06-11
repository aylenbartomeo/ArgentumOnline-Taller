#ifndef WORLD_RENDERER_H
#define WORLD_RENDERER_H

#include <string>

#include <SDL2pp/SDL2pp.hh>
#include <SDL_ttf.h>

#include "../rendering/TextureManager.h"
#include "../rendering/TileMap.h"
#include "../rendering/Viewport.h"
#include "common/include/dto/Snapshot.h"

class WorldRenderer {
public:
    WorldRenderer(TextureManager& textures, SDL2pp::Renderer& renderer, const TileMap& map,
                  TTF_Font* worldFont);

    void renderTerrain(const CameraOffset& camera) const;
    void renderOverlays(const CameraOffset& camera) const;
    void renderGroundItems(const CameraOffset& camera, const SnapshotDTO& snapshot) const;
    void renderCitizens(const CameraOffset& camera) const;

private:
    void renderTileLayer(const std::vector<std::vector<int>>& grid, const std::string& folder,
                         const CameraOffset& camera) const;
    void drawGroundAmount(const std::string& text, int tileX, int tileY,
                          const CameraOffset& camera) const;

    TextureManager& textures;
    SDL2pp::Renderer& renderer;
    const TileMap& map;
    TTF_Font* worldFont;
};

#endif
