#ifndef SCREEN_EDITOR_H
#define SCREEN_EDITOR_H

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../client/src/rendering/TextureManager.h"

#include "Camera.h"
#include "EditorLayout.h"
#include "EditorMap.h"
#include "MapRenderer.h"

#include "Font.h"
#include "MapChooser.h"
#include "Palette.h"
#include "CatalogSprites.h"
#include "BlockBrush.h"

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
    TerrainBlock selectedBlock;
    bool rightDragging;
    int lastMouseX;
    int lastMouseY;
    std::string currentMapPath;
    Uint32 savedFlashUntil;
    Uint32 mapasFlashUntil;
    Font font;
    Font smallFont;
    bool mapListOpen;
    std::vector<MapEntry> mapEntries;
    bool newMapInput;
    std::string newMapName;
    std::string mapErrorMsg;
    std::vector<int> itemOverlays;
    Palette itemPalette;
    Palette monsterPalette;
    Palette citizenPalette;
    std::string placeMsg;
    int goldAmount;
    bool amountInput;
    std::string amountText;

    void handleEvent(const SDL_Event& event, bool& running);
    void render();
    void renderPalette();
    void renderCurrentBrush();
    Palette* currentPalette();
    void placeAtCell(int col, int row);
    bool selectedIsStackable() const;
    void saveMap();
    void openMapList();
    void closeMapList();
    void renderMapList();
    void handleMapListEvent(const SDL_Event& event);
    void handleMapListClick(int mx, int my);
    int mapListIndexAt(int mx, int my) const;
    void switchToMap(const std::string& path, bool isNew);
    void beginNewMapInput();
    void cancelNewMapInput();
    void confirmNewMap();
    SDL2pp::Rect canvasViewport() const;
    SDL2pp::Point toMockup(int winX, int winY) const;

public:
    ScreenEditor();
    void run();

    ScreenEditor(const ScreenEditor&) = delete;
    ScreenEditor& operator=(const ScreenEditor&) = delete;
};

#endif
