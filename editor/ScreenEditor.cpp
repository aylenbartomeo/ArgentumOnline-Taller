#include "ScreenEditor.h"

#include <algorithm>
#include <filesystem>
#include <string>

#include "loop/ConstantRateLoop.h"

#include "CityStamp.h"
#include "MapChooser.h"
#include "MapDefaults.h"
#include "SmartEraser.h"

namespace {
constexpr int MOCKUP_W = 1450;
constexpr int MOCKUP_H = 1085;
constexpr int WINDOW_W = 1280;
constexpr int WINDOW_H = 958;
constexpr int FRAME_MS = 16;
constexpr const char* EDITOR_RES = "resources/editor/";
constexpr int PAPIRO_W = 773;
constexpr int PAPIRO_H = 618;
constexpr int PAPIRO_X = (MOCKUP_W - PAPIRO_W) / 2;
constexpr int PAPIRO_Y = (MOCKUP_H - PAPIRO_H) / 2;
constexpr int MAP_FONT_SIZE = 26;
constexpr int LIST_X = PAPIRO_X + 90;
constexpr int LIST_W = PAPIRO_W - 180;
constexpr int LIST_TITLE_Y = PAPIRO_Y + 70;
constexpr int LIST_ROWS_TOP = PAPIRO_Y + 130;
constexpr int LIST_ROW_H = 44;
constexpr LayoutRect MAP_BACK = {PAPIRO_X + PAPIRO_W - 120, PAPIRO_Y + 25, 100, 90};
constexpr int SLOT_X = 1118;
constexpr int SLOT_Y = 490;
constexpr int SLOT_STEP = 56;
constexpr int SLOT_STEP_Y = 64;
constexpr int SLOT_ICON = 32;
constexpr int ITEM_DX = 12;
constexpr int MONSTER_DX = 16;
constexpr int CITIZEN_DX = 14;
constexpr int SLOT_HILITE = 42;
constexpr int BRUSH_X = 1210;
constexpr int BRUSH_Y = 980;
constexpr int BRUSH_TOOL_DY = 8;
constexpr int BRUSH_ITEM_X = 1210;
constexpr int BRUSH_ITEM_Y = 994;
constexpr int BRUSH_MONSTER_X = 1210;
constexpr int BRUSH_MONSTER_Y = 994;
constexpr int BRUSH_CITIZEN_X = 1210;
constexpr int BRUSH_CITIZEN_Y = 994;
constexpr int SLOT_COLS = 5;
constexpr int SLOT_VISIBLE_ROWS = 6;
constexpr LayoutRect AMOUNT_FIELD = {1150, 880, 150, 40};

bool insideRect(LayoutRect r, int x, int y) {
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

TerrainBlock blockForRegion(Region region) {
    switch (region) {
        case Region::BLOCK_PLAYA:
            return TerrainBlock::BEACH;
        case Region::BLOCK_BOSQUE:
            return TerrainBlock::FOREST;
        case Region::BLOCK_DESIERTO:
            return TerrainBlock::DESERT;
        case Region::BLOCK_CIUDAD:
            return TerrainBlock::CITY;
        case Region::BLOCK_MAZMORRA:
            return TerrainBlock::DUNGEON;
        default:
            return TerrainBlock::NONE;
    }
}

Region regionForBlock(TerrainBlock block) {
    switch (block) {
        case TerrainBlock::BEACH:
            return Region::BLOCK_PLAYA;
        case TerrainBlock::FOREST:
            return Region::BLOCK_BOSQUE;
        case TerrainBlock::DESERT:
            return Region::BLOCK_DESIERTO;
        case TerrainBlock::CITY:
            return Region::BLOCK_CIUDAD;
        case TerrainBlock::DUNGEON:
            return Region::BLOCK_MAZMORRA;
        default:
            return Region::NONE;
    }
}

std::string citizenDisplayName(const std::string& type) {
    if (type == "merchant") {
        return "Comerciante";
    }
    if (type == "banker") {
        return "Banquero";
    }
    if (type == "priest") {
        return "Sacerdote";
    }
    return type;
}

std::string monsterDisplayName(const std::string& type) {
    if (type == "goblin") {
        return "Goblin";
    }
    if (type == "orc") {
        return "Orco";
    }
    if (type == "zombie") {
        return "Zombie";
    }
    if (type == "spider") {
        return "Araña";
    }
    if (type == "golem") {
        return "Golem";
    }
    if (type == "skeleton") {
        return "Esqueleto";
    }
    return type;
}

EditorMap loadMap() {
    const std::string path = MapDefaults::DEFAULT_MAP_PATH;
    if (std::filesystem::exists(path)) {
        return EditorMap(readMapFile(path));
    }
    return EditorMap(MapDefaults::WIDTH, MapDefaults::HEIGHT, MapDefaults::TILE_SIZE,
                     MapDefaults::TILESET, MapDefaults::TILESET_COLS);
}
}  // namespace

ScreenEditor::ScreenEditor():
        sdl(SDL_INIT_VIDEO),
        window("Editor de mapas - Argentum Online", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
               WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN),
        renderer(window, -1, SDL_RENDERER_ACCELERATED),
        textures(renderer),
        map(loadMap()),
        camera(canvasRect().w, canvasRect().h, MapDefaults::TILE_SIZE, map.getWidth(),
               map.getHeight()),
        mapRenderer(renderer, textures),
        canvasTarget(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MOCKUP_W,
                     MOCKUP_H),
        screen(Screen::PRINCIPAL),
        activeTool(Tool::NONE),
        selectedBlock(TerrainBlock::NONE),
        rightDragging(false),
        lastMouseX(0),
        lastMouseY(0),
        currentMapPath(MapDefaults::DEFAULT_MAP_PATH),
        savedFlashUntil(0),
        mapasFlashUntil(0),
        font(renderer, "resources/DejaVuSans-Bold.ttf", MAP_FONT_SIZE),
        smallFont(renderer, "resources/DejaVuSans-Bold.ttf", 16),
        mapListOpen(false),
        newMapInput(false),
        itemOverlays(itemOverlayIndices()),
        itemPalette(SLOT_X, SLOT_Y, SLOT_STEP, SLOT_COLS, static_cast<int>(itemOverlays.size())),
        monsterPalette(SLOT_X, SLOT_Y, SLOT_STEP, SLOT_COLS,
                       static_cast<int>(getMonsterCatalog().size())),
        citizenPalette(SLOT_X, SLOT_Y, SLOT_STEP, SLOT_COLS,
                       static_cast<int>(getCitizenCatalog().size())),
        goldAmount(1),
        amountInput(false) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
}

SDL2pp::Rect ScreenEditor::canvasViewport() const {
    LayoutRect c = canvasRect();
    return SDL2pp::Rect(c.x, c.y, c.w, c.h);
}

SDL2pp::Point ScreenEditor::toMockup(int winX, int winY) const {
    int ww = window.GetWidth();
    int wh = window.GetHeight();
    return SDL2pp::Point(winX * MOCKUP_W / ww, winY * MOCKUP_H / wh);
}

void ScreenEditor::handleEvent(const SDL_Event& event, bool& running) {
    if (event.type == SDL_QUIT) {
        running = false;
        return;
    }
    if (mapListOpen) {
        handleMapListEvent(event);
        return;
    }
    if (amountInput) {
        if (event.type == SDL_TEXTINPUT) {
            for (const char* c = event.text.text; *c != '\0'; ++c) {
                if (*c >= '0' && *c <= '9' && amountText.size() < 7) {
                    amountText += *c;
                }
            }
        } else if (event.type == SDL_KEYDOWN) {
            SDL_Keycode sym = event.key.keysym.sym;
            if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
                goldAmount = amountText.empty() ? 1 : std::max(1, std::stoi(amountText));
                amountInput = false;
                SDL_StopTextInput();
            } else if (sym == SDLK_BACKSPACE) {
                if (!amountText.empty()) {
                    amountText.pop_back();
                }
            } else if (sym == SDLK_ESCAPE) {
                amountInput = false;
                SDL_StopTextInput();
            }
        }
        return;
    }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            SDL2pp::Point p = toMockup(event.button.x, event.button.y);
            Region region = regionAtClick(screen, p.x, p.y);
            if (region == Region::GO_TERRENO || region == Region::GO_ITEMS ||
                region == Region::GO_MONSTRUOS || region == Region::GO_CIUDADANOS) {
                screen = screenForRegion(region);
                placeMsg = "";
            } else if (region == Region::BACK) {
                screen = Screen::PRINCIPAL;
                placeMsg = "";
            } else if (region == Region::GOMA) {
                activeTool = Tool::ERASER;
                selectedBlock = TerrainBlock::NONE;
            } else if (region == Region::SPAWN) {
                activeTool = Tool::SPAWN;
                selectedBlock = TerrainBlock::NONE;
            } else if (region == Region::GUARDAR) {
                saveMap();
            } else if (region == Region::MAPAS) {
                openMapList();
                mapasFlashUntil = SDL_GetTicks() + 700;
            } else if (blockForRegion(region) != TerrainBlock::NONE) {
                selectedBlock = blockForRegion(region);
                activeTool = Tool::NONE;
                placeMsg = "";
            } else if (selectedIsStackable() && insideRect(AMOUNT_FIELD, p.x, p.y)) {
                amountInput = true;
                amountText = "";
                SDL_StartTextInput();
            } else if (currentPalette() != nullptr && p.x >= SLOT_X &&
                       p.x < SLOT_X + SLOT_COLS * SLOT_STEP && p.y >= SLOT_Y &&
                       p.y < SLOT_Y + SLOT_VISIBLE_ROWS * SLOT_STEP_Y) {
                Palette* pal = currentPalette();
                int col = (p.x - SLOT_X) / SLOT_STEP;
                int row = (p.y - SLOT_Y) / SLOT_STEP_Y + pal->getScrollRow();
                pal->setSelectedTile(row * SLOT_COLS + col);
                activeTool = Tool::NONE;
            } else if (region == Region::CANVAS && activeTool != Tool::NONE) {
                LayoutRect c = canvasRect();
                Position cell = camera.screenToCell(p.x - c.x, p.y - c.y);
                if (activeTool == Tool::ERASER) {
                    smartEraseAt(map, cell.x, cell.y);
                } else if (activeTool == Tool::SPAWN) {
                    map.setSpawn(cell.x, cell.y);
                }
            } else if (region == Region::CANVAS && activeTool == Tool::NONE &&
                       selectedBlock != TerrainBlock::NONE) {
                LayoutRect c = canvasRect();
                Position cell = camera.screenToCell(p.x - c.x, p.y - c.y);
                std::string error = blockStampError(map, selectedBlock, cell.x, cell.y);
                if (error.empty()) {
                    applyBlock(map, selectedBlock, cell.x, cell.y);
                    placeMsg = "";
                } else {
                    placeMsg = error;
                }
            } else if (region == Region::CANVAS && activeTool == Tool::NONE &&
                       currentPalette() != nullptr) {
                LayoutRect c = canvasRect();
                Position cell = camera.screenToCell(p.x - c.x, p.y - c.y);
                placeAtCell(cell.x, cell.y);
            }
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            SDL2pp::Point p = toMockup(event.button.x, event.button.y);
            rightDragging = true;
            lastMouseX = p.x;
            lastMouseY = p.y;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
            rightDragging = false;
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        if (rightDragging) {
            SDL2pp::Point p = toMockup(event.motion.x, event.motion.y);
            camera.move(lastMouseX - p.x, lastMouseY - p.y);
            lastMouseX = p.x;
            lastMouseY = p.y;
        }
    } else if (event.type == SDL_KEYDOWN) {
        SDL_Keycode key = event.key.keysym.sym;
        if (key == SDLK_PLUS || key == SDLK_EQUALS || key == SDLK_KP_PLUS) {
            camera.zoomIn();
        } else if (key == SDLK_MINUS || key == SDLK_KP_MINUS) {
            camera.zoomOut();
        } else if (key == SDLK_LEFT) {
            camera.move(-camera.getTileSize(), 0);
        } else if (key == SDLK_RIGHT) {
            camera.move(camera.getTileSize(), 0);
        } else if (key == SDLK_UP) {
            camera.move(0, -camera.getTileSize());
        } else if (key == SDLK_DOWN) {
            camera.move(0, camera.getTileSize());
        }
    } else if (event.type == SDL_MOUSEWHEEL) {
        Palette* pal = currentPalette();
        if (pal != nullptr) {
            pal->scroll(event.wheel.y > 0 ? -1 : 1);
        }
    }
}

void ScreenEditor::render() {
    renderer.SetTarget(canvasTarget);
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    SDL2pp::Texture& bg = textures.get(std::string(EDITOR_RES) + mockupFile(screen));
    renderer.Copy(bg, SDL2pp::NullOpt, SDL2pp::Rect(0, 0, MOCKUP_W, MOCKUP_H));

    SDL2pp::Rect canvas = canvasViewport();
    renderer.SetViewport(canvas);
    mapRenderer.render(map, camera, canvas.w, canvas.h);
    renderer.SetViewport(SDL2pp::NullOpt);

    LayoutRect t = topLeftToolsRect();
    SDL2pp::Texture& tools = textures.get(std::string(EDITOR_RES) + "ToolsBar.png");
    renderer.Copy(tools, SDL2pp::NullOpt, SDL2pp::Rect(t.x, t.y, t.w, t.h));
    if (activeTool != Tool::NONE) {
        LayoutRect b = (activeTool == Tool::ERASER) ? gomaRect() : spawnRect();
        int insetLeft, insetRight, insetTop, insetBottom;
        if (activeTool == Tool::ERASER) {
            insetLeft = 8;
            insetRight = 8;
            insetTop = 26;
            insetBottom = -6;
        } else {
            insetLeft = 12;
            insetRight = 32;
            insetTop = 48;
            insetBottom = 12;
        }
        const int hx = b.x + insetLeft;
        const int hy = b.y + insetTop;
        const int hw = b.w - insetLeft - insetRight;
        const int hh = b.h - insetTop - insetBottom;
        renderer.SetDrawColor(255, 235, 0, 255);
        for (int i = 0; i < 4; ++i) {
            renderer.DrawRect(SDL2pp::Rect(hx + i, hy + i, hw - 2 * i, hh - 2 * i));
        }
    }
    if (SDL_GetTicks() < savedFlashUntil) {
        LayoutRect g = guardarRect();
        const int hx = g.x + 40;
        const int hy = g.y - 70;
        const int hw = g.w - 35;
        const int hh = g.h + 30;
        renderer.SetDrawColor(255, 235, 0, 255);
        for (int i = 0; i < 4; ++i) {
            renderer.DrawRect(SDL2pp::Rect(hx + i, hy + i, hw - 2 * i, hh - 2 * i));
        }
    }
    if (screen == Screen::TERRENO) {
        LayoutRect b = terrenoBackRect();
        SDL2pp::Texture& back = textures.get(std::string(EDITOR_RES) + "BackTerreno.png");
        renderer.Copy(back, SDL2pp::NullOpt, SDL2pp::Rect(b.x, b.y, b.w, b.h));
        if (selectedBlock != TerrainBlock::NONE) {
            LayoutRect r = blockRect(regionForBlock(selectedBlock));
            renderer.SetDrawColor(255, 235, 0, 255);
            for (int i = 0; i < 4; ++i) {
                renderer.DrawRect(SDL2pp::Rect(r.x + i, r.y + i, r.w - 2 * i, r.h - 2 * i));
            }
        }
        if (!placeMsg.empty()) {
            font.drawString(placeMsg, 70, 1000, SDL_Color{200, 60, 40, 255});
        }
    }
    renderPalette();
    renderCurrentBrush();
    if (mapListOpen) {
        renderMapList();
    }
    if (SDL_GetTicks() < mapasFlashUntil) {
        LayoutRect m = mapasRect();
        renderer.SetDrawColor(255, 235, 0, 255);
        for (int i = 0; i < 4; ++i) {
            renderer.DrawRect(SDL2pp::Rect(m.x + i, m.y + i, m.w - 2 * i, m.h - 2 * i));
        }
    }

    renderer.SetTarget();
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();
    renderer.Copy(canvasTarget, SDL2pp::NullOpt, SDL2pp::NullOpt);
    renderer.Present();
}

void ScreenEditor::saveMap() {
    writeMapFile(currentMapPath, map.toJson());
    savedFlashUntil = SDL_GetTicks() + 700;
}

void ScreenEditor::openMapList() {
    mapEntries = mapEntriesFrom(listMapFiles(MapDefaults::MAPS_DIR));
    mapListOpen = true;
}

void ScreenEditor::closeMapList() { mapListOpen = false; }

int ScreenEditor::mapListIndexAt(int mx, int my) const {
    if (mx < LIST_X - 8 || mx >= LIST_X + LIST_W + 8 || my < LIST_ROWS_TOP) {
        return -1;
    }
    int idx = (my - LIST_ROWS_TOP) / LIST_ROW_H;
    if (idx >= static_cast<int>(mapEntries.size())) {
        return -1;
    }
    return idx;
}

void ScreenEditor::handleMapListClick(int mx, int my) {
    int idx = mapListIndexAt(mx, my);
    if (idx < 0) {
        return;
    }
    const MapEntry& entry = mapEntries[idx];
    if (entry.isNew) {
        beginNewMapInput();
        return;
    }
    if (entry.path == currentMapPath) {
        closeMapList();
        return;
    }
    switchToMap(entry.path, false);
}

void ScreenEditor::switchToMap(const std::string& path, bool isNew) {
    if (isNew) {
        map = EditorMap(MapDefaults::WIDTH, MapDefaults::HEIGHT, MapDefaults::TILE_SIZE,
                        MapDefaults::TILESET, MapDefaults::TILESET_COLS);
        currentMapPath = path;
        camera.setMapSize(map.getWidth(), map.getHeight());
        saveMap();
    } else {
        map = EditorMap(readMapFile(path));
        currentMapPath = path;
        camera.setMapSize(map.getWidth(), map.getHeight());
    }
    closeMapList();
}

void ScreenEditor::handleMapListEvent(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        SDL2pp::Point p = toMockup(event.button.x, event.button.y);
        if (insideRect(MAP_BACK, p.x, p.y)) {
            if (newMapInput) {
                cancelNewMapInput();
            } else {
                closeMapList();
            }
        } else if (!newMapInput) {
            handleMapListClick(p.x, p.y);
        }
    } else if (event.type == SDL_TEXTINPUT) {
        if (newMapInput) {
            newMapName += event.text.text;
        }
    } else if (event.type == SDL_KEYDOWN) {
        SDL_Keycode sym = event.key.keysym.sym;
        if (newMapInput) {
            if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
                confirmNewMap();
            } else if (sym == SDLK_BACKSPACE) {
                if (!newMapName.empty()) {
                    newMapName.pop_back();
                }
            } else if (sym == SDLK_ESCAPE) {
                cancelNewMapInput();
            }
        } else if (sym == SDLK_ESCAPE) {
            closeMapList();
        }
    }
}

void ScreenEditor::renderMapList() {
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(0, 0, 0, 150);
    renderer.FillRect(SDL2pp::Rect(0, 0, MOCKUP_W, MOCKUP_H));
    renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);

    SDL2pp::Texture& papiro = textures.get(std::string(EDITOR_RES) + "papiro.bmp");
    renderer.Copy(papiro, SDL2pp::NullOpt, SDL2pp::Rect(PAPIRO_X, PAPIRO_Y, PAPIRO_W, PAPIRO_H));

    const SDL_Color textColor = {70, 45, 20, 255};
    const SDL_Color accent = {150, 90, 20, 255};
    SDL2pp::Texture& back = textures.get(std::string(EDITOR_RES) + "BackTerreno.png");

    if (newMapInput) {
        font.drawString("Nombre del mapa nuevo (Enter confirma, Esc cancela):", LIST_X,
                        LIST_TITLE_Y, textColor);
        font.drawString(newMapName + "_", LIST_X, LIST_ROWS_TOP, accent);
        if (!mapErrorMsg.empty()) {
            font.drawString(mapErrorMsg, LIST_X, LIST_ROWS_TOP + LIST_ROW_H, textColor);
        }
        renderer.Copy(back, SDL2pp::NullOpt,
                      SDL2pp::Rect(MAP_BACK.x, MAP_BACK.y, MAP_BACK.w, MAP_BACK.h));
        return;
    }

    font.drawString("Elegí un mapa (Esc cierra)", LIST_X, LIST_TITLE_Y, textColor);
    for (size_t i = 0; i < mapEntries.size(); ++i) {
        int ry = LIST_ROWS_TOP + static_cast<int>(i) * LIST_ROW_H;
        const MapEntry& e = mapEntries[i];
        if (e.path == currentMapPath) {
            renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
            renderer.SetDrawColor(220, 180, 40, 110);
            renderer.FillRect(SDL2pp::Rect(LIST_X - 8, ry, LIST_W + 16, LIST_ROW_H - 4));
            renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
        }
        std::string label = e.isNew ? "+ Nuevo mapa" : e.displayName;
        SDL_Color color = e.isNew ? accent : textColor;
        font.drawString(label, LIST_X, ry + 4, color);
    }
    renderer.Copy(back, SDL2pp::NullOpt, SDL2pp::Rect(MAP_BACK.x, MAP_BACK.y, MAP_BACK.w, MAP_BACK.h));
}

void ScreenEditor::beginNewMapInput() {
    newMapInput = true;
    newMapName = "";
    mapErrorMsg = "";
    SDL_StartTextInput();
}

void ScreenEditor::cancelNewMapInput() {
    newMapInput = false;
    newMapName = "";
    mapErrorMsg = "";
    SDL_StopTextInput();
}

void ScreenEditor::confirmNewMap() {
    std::string error = newMapError(newMapName, listMapFiles(MapDefaults::MAPS_DIR));
    if (!error.empty()) {
        mapErrorMsg = error;
        return;
    }
    std::string path = mapPathForName(newMapName);
    newMapInput = false;
    SDL_StopTextInput();
    switchToMap(path, true);
}

Palette* ScreenEditor::currentPalette() {
    if (screen == Screen::ITEMS) {
        return &itemPalette;
    }
    if (screen == Screen::MONSTRUOS) {
        return &monsterPalette;
    }
    if (screen == Screen::CIUDADANOS) {
        return &citizenPalette;
    }
    return nullptr;
}

bool ScreenEditor::selectedIsStackable() const {
    if (screen != Screen::ITEMS) {
        return false;
    }
    return getOverlayRegistry()[itemOverlays[itemPalette.getSelectedTile()]].stackable;
}

void ScreenEditor::placeAtCell(int col, int row) {
    placeMsg = "";
    if (screen == Screen::ITEMS) {
        int idx = itemOverlays[itemPalette.getSelectedTile()];
        int amount = getOverlayRegistry()[idx].stackable ? goldAmount : 1;
        map.setItem(col, row, idx, amount);
    } else if (screen == Screen::MONSTRUOS) {
        const std::string type = getMonsterCatalog()[monsterPalette.getSelectedTile()].type;
        std::string error = monsterPlacementError(map, col, row);
        if (error.empty()) {
            map.addMonster(type, col, row);
        } else {
            placeMsg = error;
        }
    } else if (screen == Screen::CIUDADANOS) {
        const std::string type = getCitizenCatalog()[citizenPalette.getSelectedTile()].type;
        std::string error = citizenPlacementError(map, type, col, row);
        if (error.empty()) {
            map.addCitizen(type, col, row);
        } else {
            placeMsg = error;
        }
    }
}

void ScreenEditor::renderPalette() {
    const Palette* pal = nullptr;
    if (screen == Screen::ITEMS) {
        pal = &itemPalette;
    } else if (screen == Screen::MONSTRUOS) {
        pal = &monsterPalette;
    } else if (screen == Screen::CIUDADANOS) {
        pal = &citizenPalette;
    }
    if (pal == nullptr) {
        return;
    }
    const int ts = pal->getTileDrawSize();
    for (int i = 0; i < pal->getTileCount(); ++i) {
        int col = i % pal->getCols();
        int row = i / pal->getCols() - pal->getScrollRow();
        if (row < 0 || row >= SLOT_VISIBLE_ROWS) {
            continue;
        }
        int dx = pal->getPanelX() + col * ts;
        int dy = pal->getPanelY() + row * SLOT_STEP_Y;
        int iy = dy + (SLOT_STEP - SLOT_ICON) / 2;
        if (screen == Screen::ITEMS) {
            drawOverlaySprite(renderer, textures, getOverlayRegistry()[itemOverlays[i]],
                              dx + ITEM_DX, iy, SLOT_ICON);
        } else if (screen == Screen::MONSTRUOS) {
            drawMonsterSprite(renderer, textures, getMonsterCatalog()[i], dx + MONSTER_DX, iy,
                              SLOT_ICON);
        } else {
            drawCitizenSprite(renderer, textures, getCitizenCatalog()[i], dx + CITIZEN_DX, iy,
                              SLOT_ICON);
        }
        if (i == pal->getSelectedTile()) {
            renderer.SetDrawColor(255, 235, 0, 255);
            int hx = dx + (SLOT_STEP - SLOT_HILITE) / 2;
            int hy = dy + (SLOT_STEP - SLOT_HILITE) / 2;
            for (int k = 0; k < 3; ++k) {
                renderer.DrawRect(
                        SDL2pp::Rect(hx + k, hy + k, SLOT_HILITE - 2 * k, SLOT_HILITE - 2 * k));
            }
        }
    }
    if (!placeMsg.empty()) {
        font.drawString(placeMsg, 70, 1000, SDL_Color{200, 60, 40, 255});
    }
    if (selectedIsStackable()) {
        renderer.SetDrawColor(20, 20, 20, 255);
        renderer.FillRect(
                SDL2pp::Rect(AMOUNT_FIELD.x, AMOUNT_FIELD.y, AMOUNT_FIELD.w, AMOUNT_FIELD.h));
        std::string txt =
                amountInput ? (amountText + "_") : ("Cantidad: " + std::to_string(goldAmount));
        font.drawString(txt, AMOUNT_FIELD.x + 6, AMOUNT_FIELD.y + 6, SDL_Color{240, 220, 120, 255});
    }
}

void ScreenEditor::renderCurrentBrush() {
    if (screen == Screen::TERRENO) {
        return;
    }
    SDL_Color gold{240, 220, 120, 255};
    if (activeTool == Tool::ERASER) {
        smallFont.drawString("Goma", BRUSH_X, BRUSH_Y + BRUSH_TOOL_DY, gold);
        return;
    }
    if (activeTool == Tool::SPAWN) {
        smallFont.drawString("Spawn", BRUSH_X, BRUSH_Y + BRUSH_TOOL_DY, gold);
        return;
    }
    Palette* pal = currentPalette();
    if (pal == nullptr) {
        return;
    }
    int sel = pal->getSelectedTile();
    std::string name;
    int bx;
    int by;
    if (screen == Screen::ITEMS) {
        name = getOverlayRegistry()[itemOverlays[sel]].name;
        bx = BRUSH_ITEM_X;
        by = BRUSH_ITEM_Y;
    } else if (screen == Screen::MONSTRUOS) {
        name = monsterDisplayName(getMonsterCatalog()[sel].type);
        bx = BRUSH_MONSTER_X;
        by = BRUSH_MONSTER_Y;
    } else {
        name = citizenDisplayName(getCitizenCatalog()[sel].type);
        bx = BRUSH_CITIZEN_X;
        by = BRUSH_CITIZEN_Y;
    }
    smallFont.drawString(name, bx, by, gold);
}

void ScreenEditor::run() {
    bool running = true;
    ConstantRateLoop loop(FRAME_MS);
    loop.run([this, &running](int64_t) -> bool {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleEvent(event, running);
        }
        render();
        return running;
    });
}
