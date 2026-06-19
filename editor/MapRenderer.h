#ifndef MAP_RENDERER_H
#define MAP_RENDERER_H

#include <SDL2pp/SDL2pp.hh>

#include "../client/src/rendering/TextureManager.h"

#include "Camera.h"
#include "EditorMap.h"

class MapRenderer {
private:
    SDL2pp::Renderer& renderer;
    TextureManager& textures;

    void drawGroundLayer(const EditorMap& map, const Camera& camera, int canvasW, int canvasH,
                         const std::vector<std::vector<int>>& grid);
    void drawDecorationLayer(const EditorMap& map, const Camera& camera, int canvasW, int canvasH);
    void drawItems(const EditorMap& map, const Camera& camera, int canvasW, int canvasH);
    void drawMonsters(const EditorMap& map, const Camera& camera, int canvasW, int canvasH);
    void drawCitizens(const EditorMap& map, const Camera& camera, int canvasW, int canvasH);
    void drawSpawn(const EditorMap& map, const Camera& camera);

public:
    MapRenderer(SDL2pp::Renderer& renderer, TextureManager& textures);
    void render(const EditorMap& map, const Camera& camera, int canvasW, int canvasH);
};

#endif
