#ifndef EDITOR_H
#define EDITOR_H

#include <string>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "OverlayRegistry.h"

#include "Camera.h"
#include "EditorMap.h"
#include "Font.h"
#include "Palette.h"
#include "SpawnCatalogs.h"
#include "TextureManager.h"
#include "Toolbar.h"

class Editor {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    TextureManager textures;
    EditorMap map;
    Camera camera;
    Palette overlayPalette;
    Palette monsterPalette;
    Palette citizenPalette;
    Font font;
    Toolbar toolbar;
    std::string mapPath;
    bool rightDragging;
    int lastMouseX;
    int lastMouseY;
    bool dirty;
    Uint32 savedFlashUntil;

    void handleEvent(const SDL_Event& event, bool& running);
    void handleLeftClick(int x, int y);
    void handlePanelClick(int x, int y);
    void save();
    void updateTitle();

    void render();
    void renderTerrain();
    void renderOverlays();
    void renderMonsters();
    void renderCitizens();
    void renderSpawn();
    void renderPanel();
    void renderStatusBar();
    void drawGrass(int dstX, int dstY, int dstSize);
    void drawDarkGrass(int dstX, int dstY, int dstSize);
    bool cellInSafeZone(int col, int row) const;
    void drawOverlay(const OverlayDef& def, int cellX, int cellY, int cellSize);
    void drawMonsterFromCatalog(const MonsterCatalogEntry& entry, int cellX, int cellY,
                                int cellSize);
    void drawCitizenFromCatalog(const CitizenCatalogEntry& entry, int cellX, int cellY,
                                int cellSize);
    void drawCharacter(int dstX, int dstY, int dstW, int dstH);
    void drawSaveIcon(const Toolbar::Button& b);
    void drawEraserIcon(const Toolbar::Button& b);
    Palette& activePalette();
    const Palette& activePalette() const;

public:
    Editor(EditorMap initialMap, const std::string& mapPath);
    void run();

    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
};

#endif
