#ifndef EDITOR_H
#define EDITOR_H

#include <string>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "TextureManager.h"

#include "Camera.h"
#include "EditorMap.h"
#include "Palette.h"
#include "Toolbar.h"

class Editor {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    TextureManager textures;
    EditorMap map;
    Camera camera;
    Palette palette;
    Toolbar toolbar;
    std::string mapPath;
    bool rightDragging;
    int lastMouseX;
    int lastMouseY;
    bool dirty;
    Uint32 savedFlashUntil;

    int tileCountFromTexture();

    void handleEvent(const SDL_Event& event, bool& running);
    void handleLeftClick(int x, int y);
    void handlePanelClick(int x, int y);
    void save();
    void updateTitle();

    void render();
    void renderTerrain();
    void renderSpawn();
    void renderPanel();
    void renderStatusBar();
    void drawTile(int tileId, int dstX, int dstY, int dstSize);
    void drawCharacter(int dstX, int dstY, int dstW, int dstH);
    void drawMinus(const Toolbar::Button& b);
    void drawPlus(const Toolbar::Button& b);
    void drawSaveIcon(const Toolbar::Button& b);

public:
    Editor(EditorMap initialMap, const std::string& mapPath);
    void run();

    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
};

#endif
