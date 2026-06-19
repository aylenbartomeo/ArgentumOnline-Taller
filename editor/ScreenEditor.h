#ifndef SCREEN_EDITOR_H
#define SCREEN_EDITOR_H

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../client/src/rendering/TextureManager.h"

#include "Camera.h"
#include "EditorLayout.h"
#include "EditorMap.h"
#include "MapRenderer.h"

enum class Tool { NONE, ERASER, SPAWN };

class ScreenEditor {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    TextureManager textures;
    EditorMap map;
    Camera camera;
    MapRenderer mapRenderer;
    SDL2pp::Texture canvasTarget;
    Screen screen;
    Tool activeTool;
    bool rightDragging;
    int lastMouseX;
    int lastMouseY;
    std::string currentMapPath;
    Uint32 savedFlashUntil;

    void handleEvent(const SDL_Event& event, bool& running);
    void render();
    void saveMap();
    SDL2pp::Rect canvasViewport() const;
    SDL2pp::Point toMockup(int winX, int winY) const;

public:
    ScreenEditor();
    void run();

    ScreenEditor(const ScreenEditor&) = delete;
    ScreenEditor& operator=(const ScreenEditor&) = delete;
};

#endif
