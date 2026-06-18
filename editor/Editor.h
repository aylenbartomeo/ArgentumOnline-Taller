#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../client/src/rendering/TextureManager.h"

#include "Camera.h"
#include "EditorMap.h"
#include "Font.h"
#include "OverlayRegistry.h"
#include "MapChooser.h"
#include "Palette.h"
#include "SpawnCatalogs.h"
#include "Toolbar.h"

class Editor {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    TextureManager textures;
    EditorMap map;
    Camera camera;
    std::vector<int> itemOverlays;
    Palette itemPalette;
    Palette monsterPalette;
    Palette citizenPalette;
    Font font;
    Toolbar toolbar;
    std::string mapPath;
    std::string statusMsg;
    bool rightDragging;
    int lastMouseX;
    int lastMouseY;
    bool dirty;
    Uint32 savedFlashUntil;
    bool mapListOpen;
    std::vector<MapEntry> mapEntries;
    bool newMapInput;
    std::string newMapName;

    void handleEvent(const SDL_Event& event, bool& running);
    void handleLeftClick(int x, int y);
    void handlePanelClick(int x, int y);
    void save();
    void updateTitle();
    void openMapList();
    void closeMapList();
    void switchToMap(const std::string& path, bool isNew);
    void handleMapListClick(int x, int y);
    int mapListIndexAt(int x, int y) const;
    void beginNewMapInput();
    void cancelNewMapInput();
    void confirmNewMap();

    void render();
    void renderTerrain();
    void renderSafeZones();
    void renderDungeons();
    void renderCitizenZones();
    void renderItems();
    void renderMonsters();
    void renderCitizens();
    void renderSpawn();
    void renderPanel();
    void renderStatusBar();
    void renderMapList();
    void drawGroundLayer(const std::vector<std::vector<int>>& grid);
    void drawDecorationLayer();
    void drawGrass(int dstX, int dstY, int dstSize);
    void drawOverlay(const OverlayDef& def, int cellX, int cellY, int cellSize);
    void drawMonsterFromCatalog(const MonsterCatalogEntry& entry, int cellX, int cellY,
                                int cellSize);
    void drawCitizenFromCatalog(const CitizenCatalogEntry& entry, int cellX, int cellY,
                                int cellSize);
    void drawCharacter(int dstX, int dstY, int dstW, int dstH);
    void drawSaveIcon(const Toolbar::Button& b);
    void drawEraserIcon(const Toolbar::Button& b);
    void drawMapsIcon(const Toolbar::Button& b);
    void drawDungeonIcon(const Toolbar::Button& b);
    Palette& activePalette();
    const Palette& activePalette() const;

public:
    Editor(EditorMap initialMap, const std::string& mapPath);
    void run();

    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
};

#endif
