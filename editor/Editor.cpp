#include "Editor.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

#include "loop/ConstantRateLoop.h"

#include "CityStamp.h"
#include "DungeonStamp.h"
#include "MapChooser.h"
#include "MapDefaults.h"

namespace {
constexpr int FRAME_DURATION_MS = 16;
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr int PANEL_WIDTH = 200;
constexpr int STATUS_HEIGHT = 40;
constexpr int CANVAS_WIDTH = WINDOW_WIDTH - PANEL_WIDTH;
constexpr int CANVAS_HEIGHT = WINDOW_HEIGHT - STATUS_HEIGHT;
constexpr int TILE_NATIVE = 32;
constexpr int PANEL_X = CANVAS_WIDTH;
constexpr int CAMERA_STEP = 32;

constexpr int BTN_W = PANEL_WIDTH - 20;
constexpr int BTN_H = 34;
constexpr int BTN_X = PANEL_X + 10;
constexpr int OVERLAY_BTN_Y = 6;
constexpr int MONSTER_BTN_Y = 44;
constexpr int CITIZEN_BTN_Y = 82;
constexpr int CITY_BTN_Y = 120;
constexpr int CITY_ERASE_BTN_Y = 158;
constexpr int DUNGEON_BTN_Y = 196;
constexpr int DUNGEON_ERASE_BTN_Y = 234;
constexpr int ERASER_BTN_Y = 272;
constexpr int SPAWN_BTN_Y = 310;
constexpr int SAVE_Y = 348;
constexpr int MAPS_Y = 386;

constexpr int PALETTE_X = PANEL_X + 10;
constexpr int PALETTE_Y = 424;
constexpr int PALETTE_TILE = 32;
constexpr int PALETTE_COLS = 5;

constexpr int MAPLIST_X = 40;
constexpr int MAPLIST_Y = 40;
constexpr int MAPLIST_W = CANVAS_WIDTH - 80;
constexpr int MAPLIST_ROW_H = 32;
constexpr int MAPLIST_TITLE_H = 36;

constexpr const char* RESOURCES_DIR = "resources/";
constexpr const char* FONT_TTF_PATH = "resources/DejaVuSans-Bold.ttf";
constexpr int LABEL_FONT_SIZE = 14;
constexpr SDL_Color LABEL_COLOR = {240, 240, 240, 255};
constexpr const char* GRASS_SHEET_PATH = "resources/5108.png";
constexpr int GRASS_SRC_X = 416;
constexpr int GRASS_SRC_Y = 384;
constexpr int GRASS_SRC_SIZE = 32;
constexpr const char* CHARACTER_SHEET_PATH = "resources/1500.png";
constexpr int CHARACTER_FRAME_X = 2;
constexpr int CHARACTER_FRAME_Y = 4;
constexpr int CHARACTER_FRAME_W = 24;
constexpr int CHARACTER_FRAME_H = 44;
constexpr double TAU = 6.283185307179586;
constexpr int MARKER_SEGMENTS = 24;
constexpr int MARKER_SHIFT_X = 3;

constexpr const char* HEAD_SHEET_PATH = "resources/420.png";
constexpr int HEAD_FRAME_X = 6;
constexpr int HEAD_FRAME_Y = 13;
constexpr int HEAD_FRAME_W = 13;
constexpr int HEAD_FRAME_H = 15;
constexpr int HEAD_DRAW_W = 18;
constexpr int HEAD_DRAW_H = 20;
constexpr int HEAD_OVERLAP = 6;

std::string labelForTool(Tool tool) {
    switch (tool) {
        case Tool::OVERLAY:
            return "Items";
        case Tool::MONSTER:
            return "Monstruo";
        case Tool::CITIZEN:
            return "Citizen";
        case Tool::CITY:
            return "Ciudad";
        case Tool::CITY_ERASE:
            return "Borrar ciudad";
        case Tool::DUNGEON:
            return "Mazmorra";
        case Tool::DUNGEON_ERASE:
            return "Borrar mazmorra";
        case Tool::ERASER:
            return "Goma";
        case Tool::SPAWN:
            return "Spawn";
    }
    return "";
}

std::vector<int> buildItemOverlays() {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    std::vector<int> indices;
    for (size_t i = 0; i < registry.size(); ++i) {
        if (registry[i].itemId != 0) {
            indices.push_back(static_cast<int>(i));
        }
    }
    return indices;
}
}  // namespace

Editor::Editor(EditorMap initialMap, const std::string& mapPath):
        sdl(SDL_INIT_VIDEO),
        window("Editor de mapas - Argentum Online", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
               WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN),
        renderer(window, -1, SDL_RENDERER_ACCELERATED),
        textures(renderer),
        map(std::move(initialMap)),
        camera(CANVAS_WIDTH, CANVAS_HEIGHT, TILE_NATIVE, map.getWidth(), map.getHeight()),
        itemOverlays(buildItemOverlays()),
        itemPalette(PALETTE_X, PALETTE_Y, PALETTE_TILE, PALETTE_COLS,
                    static_cast<int>(itemOverlays.size())),
        monsterPalette(PALETTE_X, PALETTE_Y, PALETTE_TILE, PALETTE_COLS,
                       static_cast<int>(getMonsterCatalog().size())),
        citizenPalette(PALETTE_X, PALETTE_Y, PALETTE_TILE, PALETTE_COLS,
                       static_cast<int>(getCitizenCatalog().size())),
        font(renderer, FONT_TTF_PATH, LABEL_FONT_SIZE),
        toolbar(),
        mapPath(mapPath),
        rightDragging(false),
        lastMouseX(0),
        lastMouseY(0),
        dirty(false),
        savedFlashUntil(0),
        mapListOpen(false),
        newMapInput(false) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    toolbar.addToolButton(BTN_X, OVERLAY_BTN_Y, BTN_W, BTN_H, Tool::OVERLAY);
    toolbar.addToolButton(BTN_X, MONSTER_BTN_Y, BTN_W, BTN_H, Tool::MONSTER);
    toolbar.addToolButton(BTN_X, CITIZEN_BTN_Y, BTN_W, BTN_H, Tool::CITIZEN);
    toolbar.addToolButton(BTN_X, CITY_BTN_Y, BTN_W, BTN_H, Tool::CITY);
    toolbar.addToolButton(BTN_X, CITY_ERASE_BTN_Y, BTN_W, BTN_H, Tool::CITY_ERASE);
    toolbar.addToolButton(BTN_X, DUNGEON_BTN_Y, BTN_W, BTN_H, Tool::DUNGEON);
    toolbar.addToolButton(BTN_X, DUNGEON_ERASE_BTN_Y, BTN_W, BTN_H, Tool::DUNGEON_ERASE);
    toolbar.addToolButton(BTN_X, ERASER_BTN_Y, BTN_W, BTN_H, Tool::ERASER);
    toolbar.addToolButton(BTN_X, SPAWN_BTN_Y, BTN_W, BTN_H, Tool::SPAWN);
    toolbar.addActionButton(BTN_X, SAVE_Y, BTN_W, BTN_H, ToolbarAction::SAVE);
    toolbar.addActionButton(BTN_X, MAPS_Y, BTN_W, BTN_H, ToolbarAction::OPEN_MAPS);

    updateTitle();
}

void Editor::run() {
    bool running = true;
    ConstantRateLoop loop(FRAME_DURATION_MS);
    loop.run([this, &running](int64_t) -> bool {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleEvent(event, running);
        }
        render();
        return running;
    });
}

void Editor::handleEvent(const SDL_Event& event, bool& running) {
    if (event.type == SDL_QUIT) {
        running = false;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            handleLeftClick(event.button.x, event.button.y);
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            rightDragging = true;
            lastMouseX = event.button.x;
            lastMouseY = event.button.y;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
            rightDragging = false;
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        if (rightDragging) {
            camera.move(lastMouseX - event.motion.x, lastMouseY - event.motion.y);
            lastMouseX = event.motion.x;
            lastMouseY = event.motion.y;
        }
    } else if (event.type == SDL_MOUSEWHEEL) {
        int mouseX = 0;
        int mouseY = 0;
        SDL_GetMouseState(&mouseX, &mouseY);
        if (mouseX >= PANEL_X) {
            activePalette().scroll(-event.wheel.y);
        }
    } else if (event.type == SDL_TEXTINPUT) {
        if (newMapInput) {
            newMapName += event.text.text;
        }
    } else if (event.type == SDL_KEYDOWN) {
        if (newMapInput) {
            SDL_Keycode sym = event.key.keysym.sym;
            if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
                confirmNewMap();
            } else if (sym == SDLK_BACKSPACE) {
                if (!newMapName.empty()) {
                    newMapName.pop_back();
                }
            } else if (sym == SDLK_ESCAPE) {
                cancelNewMapInput();
            }
            return;
        }
        if (mapListOpen) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                closeMapList();
            }
            return;
        }
        switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                camera.move(-CAMERA_STEP, 0);
                break;
            case SDLK_RIGHT:
                camera.move(CAMERA_STEP, 0);
                break;
            case SDLK_UP:
                camera.move(0, -CAMERA_STEP);
                break;
            case SDLK_DOWN:
                camera.move(0, CAMERA_STEP);
                break;
            case SDLK_PLUS:
            case SDLK_EQUALS:
            case SDLK_KP_PLUS:
                camera.zoomIn();
                break;
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
                camera.zoomOut();
                break;
            case SDLK_s:
                save();
                break;
            default:
                break;
        }
    }
}

void Editor::handleLeftClick(int x, int y) {
    if (newMapInput) {
        return;
    }
    if (mapListOpen) {
        handleMapListClick(x, y);
        return;
    }
    if (x < CANVAS_WIDTH && y < CANVAS_HEIGHT) {
        Position cell = camera.screenToCell(x, y);
        if (cell.x >= 0 && cell.x < map.getWidth() && cell.y >= 0 && cell.y < map.getHeight()) {
            switch (toolbar.getActiveTool()) {
                case Tool::OVERLAY:
                    map.paintItem(cell.x, cell.y, itemOverlays[itemPalette.getSelectedTile()]);
                    statusMsg = "";
                    break;
                case Tool::MONSTER: {
                    const std::string type =
                            getMonsterCatalog()[monsterPalette.getSelectedTile()].type;
                    std::string error = monsterPlacementError(map, cell.x, cell.y);
                    if (error.empty()) {
                        map.addMonster(type, cell.x, cell.y);
                        statusMsg = "";
                    } else {
                        statusMsg = error;
                    }
                    break;
                }
                case Tool::CITIZEN: {
                    const std::string type =
                            getCitizenCatalog()[citizenPalette.getSelectedTile()].type;
                    std::string error = citizenPlacementError(map, type, cell.x, cell.y);
                    if (error.empty()) {
                        map.addCitizen(type, cell.x, cell.y);
                        statusMsg = "";
                    } else {
                        statusMsg = error;
                    }
                    break;
                }
                case Tool::CITY: {
                    const CellPos origin = cityOriginForClick(cell.x, cell.y);
                    std::string error = cityStampError(map, origin.x, origin.y);
                    if (error.empty()) {
                        applyCityPrefab(map, origin.x, origin.y,
                                        "Ciudad " + std::to_string(map.getSafeZones().size() + 1));
                        statusMsg = "";
                    } else {
                        statusMsg = error;
                    }
                    break;
                }
                case Tool::CITY_ERASE: {
                    if (eraseCityAt(map, cell.x, cell.y)) {
                        statusMsg = "";
                    } else {
                        statusMsg = "ahi no hay ninguna ciudad";
                    }
                    break;
                }
                case Tool::DUNGEON: {
                    const CellPos origin = dungeonOriginForClick(cell.x, cell.y);
                    std::string error = dungeonStampError(map, origin.x, origin.y);
                    if (error.empty()) {
                        applyDungeonPrefab(map, origin.x, origin.y);
                        statusMsg = "";
                    } else {
                        statusMsg = error;
                    }
                    break;
                }
                case Tool::DUNGEON_ERASE: {
                    if (eraseDungeonAt(map, cell.x, cell.y)) {
                        statusMsg = "";
                    } else {
                        statusMsg = "ahi no hay ninguna mazmorra";
                    }
                    break;
                }
                case Tool::ERASER:
                    map.removeEntitiesAt(cell.x, cell.y);
                    map.removeItemAt(cell.x, cell.y);
                    statusMsg = "";
                    break;
                case Tool::SPAWN:
                    map.setSpawn(cell.x, cell.y);
                    statusMsg = "";
                    break;
            }
            dirty = true;
            updateTitle();
        }
    } else if (x >= PANEL_X) {
        handlePanelClick(x, y);
    }
}

void Editor::handlePanelClick(int x, int y) {
    ToolbarAction action = toolbar.handleClick(x, y);
    switch (action) {
        case ToolbarAction::SAVE:
            save();
            break;
        case ToolbarAction::OPEN_MAPS:
            openMapList();
            break;
        case ToolbarAction::TOOL_CHANGED:
            statusMsg = "";
            break;
        case ToolbarAction::NONE:
            activePalette().selectFromClick(x, y);
            break;
    }
}

void Editor::save() {
    std::filesystem::path path(mapPath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
    std::ofstream out(mapPath);
    if (!out) {
        std::cerr << "No pude guardar el mapa en " << mapPath << std::endl;
        return;
    }
    out << map.toJson();
    dirty = false;
    savedFlashUntil = SDL_GetTicks() + 700;
    updateTitle();
    std::cout << "Mapa guardado en " << mapPath << std::endl;
}

void Editor::updateTitle() {
    std::string title = "Editor de mapas - Argentum Online";
    title += dirty ? "  [* sin guardar]" : "  [guardado]";
    window.SetTitle(title);
}

void Editor::openMapList() {
    mapEntries = mapEntriesFrom(listMapFiles(MapDefaults::MAPS_DIR));
    mapListOpen = true;
    statusMsg = "";
}

void Editor::closeMapList() { mapListOpen = false; }

void Editor::beginNewMapInput() {
    newMapInput = true;
    newMapName = "";
    statusMsg = "";
    SDL_StartTextInput();
}

void Editor::cancelNewMapInput() {
    newMapInput = false;
    newMapName = "";
    SDL_StopTextInput();
}

void Editor::confirmNewMap() {
    std::string error = newMapError(newMapName, listMapFiles(MapDefaults::MAPS_DIR));
    if (!error.empty()) {
        statusMsg = error;
        return;
    }
    std::string path = mapPathForName(newMapName);
    newMapInput = false;
    SDL_StopTextInput();
    switchToMap(path, true);
}

void Editor::switchToMap(const std::string& path, bool isNew) {
    if (isNew) {
        map = EditorMap(MapDefaults::WIDTH, MapDefaults::HEIGHT, MapDefaults::TILE_SIZE,
                        MapDefaults::TILESET, MapDefaults::TILESET_COLS);
        mapPath = path;
        camera.setMapSize(map.getWidth(), map.getHeight());
        save();
    } else {
        map = EditorMap(readMapFile(path));
        mapPath = path;
        camera.setMapSize(map.getWidth(), map.getHeight());
        dirty = false;
    }
    closeMapList();
    statusMsg = "";
    updateTitle();
}

int Editor::mapListIndexAt(int x, int y) const {
    int rowsTop = MAPLIST_Y + MAPLIST_TITLE_H;
    if (x < MAPLIST_X || x >= MAPLIST_X + MAPLIST_W || y < rowsTop) {
        return -1;
    }
    int idx = (y - rowsTop) / MAPLIST_ROW_H;
    if (idx < 0 || idx >= static_cast<int>(mapEntries.size())) {
        return -1;
    }
    return idx;
}

void Editor::handleMapListClick(int x, int y) {
    int idx = mapListIndexAt(x, y);
    if (idx < 0) {
        closeMapList();
        return;
    }
    const MapEntry& entry = mapEntries[idx];
    if (entry.isNew) {
        beginNewMapInput();
        return;
    }
    if (entry.path == mapPath) {
        closeMapList();
        return;
    }
    if (dirty) {
        statusMsg = "tenés cambios sin guardar (apretá S o Guardar)";
        return;
    }
    switchToMap(entry.path, false);
}

void Editor::renderMapList() {
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(0, 0, 0, 180);
    renderer.FillRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));

    int panelH = MAPLIST_TITLE_H + MAPLIST_ROW_H * static_cast<int>(mapEntries.size()) + 8;
    renderer.SetDrawColor(30, 30, 38, 255);
    renderer.FillRect(SDL2pp::Rect(MAPLIST_X, MAPLIST_Y, MAPLIST_W, panelH));

    if (newMapInput) {
        font.drawString("Nombre del mapa nuevo (Enter confirma, Esc cancela):", MAPLIST_X + 10,
                        MAPLIST_Y + 8, LABEL_COLOR);
        font.drawString(newMapName + "_", MAPLIST_X + 12, MAPLIST_Y + MAPLIST_TITLE_H + 6,
                        SDL_Color{255, 235, 120, 255});
        return;
    }

    font.drawString("Elegí un mapa (Esc cierra)", MAPLIST_X + 10, MAPLIST_Y + 8, LABEL_COLOR);

    int rowsTop = MAPLIST_Y + MAPLIST_TITLE_H;
    for (size_t i = 0; i < mapEntries.size(); ++i) {
        int ry = rowsTop + static_cast<int>(i) * MAPLIST_ROW_H;
        const MapEntry& e = mapEntries[i];
        if (e.path == mapPath) {
            renderer.SetDrawColor(60, 90, 60, 255);
            renderer.FillRect(SDL2pp::Rect(MAPLIST_X + 4, ry, MAPLIST_W - 8, MAPLIST_ROW_H - 2));
        }
        SDL_Color color = e.isNew ? SDL_Color{255, 235, 120, 255} : LABEL_COLOR;
        std::string label = e.isNew ? "+ Nuevo mapa" : e.displayName;
        font.drawString(label, MAPLIST_X + 12, ry + 6, color);
    }
}

Palette& Editor::activePalette() {
    switch (toolbar.getActiveTool()) {
        case Tool::MONSTER:
            return monsterPalette;
        case Tool::CITIZEN:
            return citizenPalette;
        case Tool::OVERLAY:
        case Tool::CITY:
        case Tool::CITY_ERASE:
        case Tool::DUNGEON:
        case Tool::DUNGEON_ERASE:
        case Tool::ERASER:
        case Tool::SPAWN:
        default:
            return itemPalette;
    }
}

const Palette& Editor::activePalette() const { return const_cast<Editor*>(this)->activePalette(); }

void Editor::drawGrass(int dstX, int dstY, int dstSize) {
    SDL2pp::Texture& tex = textures.get(GRASS_SHEET_PATH);
    const SDL2pp::Rect srcRect(GRASS_SRC_X, GRASS_SRC_Y, GRASS_SRC_SIZE, GRASS_SRC_SIZE);
    const SDL2pp::Rect dstRect(dstX, dstY, dstSize, dstSize);
    renderer.Copy(tex, srcRect, dstRect);
}

void Editor::drawGroundLayer(const std::vector<std::vector<int>>& grid) {
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
            if (screen.x + dstW <= 0 || screen.x >= CANVAS_WIDTH || screen.y + dstH <= 0 ||
                screen.y >= CANVAS_HEIGHT) {
                continue;
            }
            renderer.Copy(tex, SDL2pp::Rect(0, 0, tex.GetWidth(), tex.GetHeight()),
                          SDL2pp::Rect(screen.x, screen.y, dstW, dstH));
        }
    }
}

void Editor::drawDecorationLayer() {
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
            if (screen.x + dstW <= 0 || screen.x >= CANVAS_WIDTH || dstY + dstH <= 0 ||
                dstY >= CANVAS_HEIGHT) {
                continue;
            }
            renderer.Copy(tex, SDL2pp::Rect(0, 0, tex.GetWidth(), tex.GetHeight()),
                          SDL2pp::Rect(screen.x, dstY, dstW, dstH));
        }
    }
}

void Editor::drawOverlay(const OverlayDef& def, int cellX, int cellY, int cellSize) {
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

void Editor::drawMonsterFromCatalog(const MonsterCatalogEntry& entry, int cellX, int cellY,
                                    int cellSize) {
    SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + entry.sheet);
    const SDL2pp::Rect srcRect(entry.srcX, entry.srcY, entry.srcW, entry.srcH);
    const int box = cellSize * 2;
    int dstW = box;
    int dstH = entry.srcH * box / entry.srcW;
    if (dstH > box) {
        dstH = box;
        dstW = entry.srcW * box / entry.srcH;
    }
    const int dstX = cellX + (cellSize - dstW) / 2;
    const int dstY = cellY + cellSize - dstH;
    renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));

    if (entry.drawHead) {
        SDL2pp::Texture& headTex = textures.get(std::string(RESOURCES_DIR) + entry.headSheet);
        const int headW = HEAD_DRAW_W * cellSize / TILE_NATIVE;
        const int headH = HEAD_DRAW_H * cellSize / TILE_NATIVE;
        const int headX = cellX + (cellSize - headW) / 2;
        const int headY = dstY + entry.headOverlap * cellSize / TILE_NATIVE - headH;
        renderer.Copy(headTex, SDL2pp::Rect(entry.headX, entry.headY, entry.headW, entry.headH),
                      SDL2pp::Rect(headX, headY, headW, headH));
    }
}

void Editor::drawCitizenFromCatalog(const CitizenCatalogEntry& entry, int cellX, int cellY,
                                    int cellSize) {
    SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + entry.sheet);
    const SDL2pp::Rect srcRect(entry.srcX, entry.srcY, entry.srcW, entry.srcH);
    const int dstW = cellSize;
    const int dstH = (entry.srcH * cellSize) / entry.srcW;
    const int dstX = cellX;
    const int dstY = cellY + cellSize - dstH;
    renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));

    SDL2pp::Texture& headTex = textures.get(std::string(RESOURCES_DIR) + entry.headSheet);
    const int headW = HEAD_DRAW_W * cellSize / TILE_NATIVE;
    const int headH = HEAD_DRAW_H * cellSize / TILE_NATIVE;
    const int headX = cellX + (cellSize - headW) / 2;
    const int headY = dstY + entry.headOverlap * cellSize / TILE_NATIVE - headH;
    renderer.Copy(headTex, SDL2pp::Rect(entry.headX, entry.headY, entry.headW, entry.headH),
                  SDL2pp::Rect(headX, headY, headW, headH));
}

void Editor::drawCharacter(int dstX, int dstY, int dstW, int dstH) {
    SDL2pp::Texture& tex = textures.get(CHARACTER_SHEET_PATH);
    SDL2pp::Rect srcRect(CHARACTER_FRAME_X, CHARACTER_FRAME_Y, CHARACTER_FRAME_W,
                         CHARACTER_FRAME_H);
    SDL2pp::Rect dstRect(dstX, dstY, dstW, dstH);
    renderer.Copy(tex, srcRect, dstRect);
}

void Editor::render() {
    renderer.SetDrawColor(30, 30, 30, 255);
    renderer.Clear();
    renderTerrain();
    renderSafeZones();
    renderDungeons();
    renderCitizenZones();
    renderItems();
    renderMonsters();
    renderCitizens();
    renderSpawn();
    renderPanel();
    renderStatusBar();
    if (mapListOpen) {
        renderMapList();
    }
    renderer.Present();
}

void Editor::renderTerrain() {
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    drawGroundLayer(map.getGround());
    drawGroundLayer(map.getGround2());
    drawDecorationLayer();
    renderer.SetClipRect();
}

void Editor::renderSafeZones() {
    const int ts = camera.getTileSize();
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    renderer.SetDrawColor(0, 220, 220, 255);
    for (const EditorSafeZone& zone: map.getSafeZones()) {
        Position screen = camera.cellToScreen(zone.x, zone.y);
        renderer.DrawRect(SDL2pp::Rect(screen.x, screen.y, zone.width * ts, zone.height * ts));
    }
    renderer.SetClipRect();
}

void Editor::renderDungeons() {
    const int ts = camera.getTileSize();
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    renderer.SetDrawColor(255, 120, 40, 255);
    for (const EditorDungeon& d: map.getDungeons()) {
        Position screen = camera.cellToScreen(d.x, d.y);
        renderer.DrawRect(SDL2pp::Rect(screen.x, screen.y, d.width * ts, d.height * ts));
    }
    renderer.SetClipRect();
}

void Editor::renderCitizenZones() {
    if (toolbar.getActiveTool() != Tool::CITIZEN) {
        return;
    }
    const int ts = camera.getTileSize();
    const std::string type = getCitizenCatalog()[citizenPalette.getSelectedTile()].type;
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    for (const EditorSafeZone& zone: map.getSafeZones()) {
        CellRect rect;
        if (!cityZoneFor(zone, type, rect)) {
            continue;
        }
        Position screen = camera.cellToScreen(rect.x, rect.y);
        SDL2pp::Rect dst(screen.x, screen.y, rect.width * ts, rect.height * ts);
        renderer.SetDrawColor(80, 220, 120, 60);
        renderer.FillRect(dst);
        renderer.SetDrawColor(80, 220, 120, 200);
        renderer.DrawRect(dst);
    }
    renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
    renderer.SetClipRect();
}

void Editor::renderItems() {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    const int ts = camera.getTileSize();
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    for (const auto& entry: map.getItems()) {
        int col = entry.first.first;
        int row = entry.first.second;
        const PlacedItem& item = entry.second;
        Position screen = camera.cellToScreen(col, row);
        if (screen.x + ts <= 0 || screen.x >= CANVAS_WIDTH || screen.y + ts <= 0 ||
            screen.y >= CANVAS_HEIGHT) {
            continue;
        }
        const OverlayDef& def = registry[item.overlayIndex];
        drawOverlay(def, screen.x, screen.y, ts);
        if (def.stackable && item.amount > 1) {
            const std::string text = std::to_string(item.amount);
            const SDL_Color black{0, 0, 0, 255};
            const SDL_Color white{255, 255, 255, 255};
            font.drawString(text, screen.x + 3, screen.y + 3, black);
            font.drawString(text, screen.x + 2, screen.y + 2, white);
        }
    }
    renderer.SetClipRect();
}

void Editor::renderMonsters() {
    const auto& catalog = getMonsterCatalog();
    const int ts = camera.getTileSize();
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    for (const MonsterSpawn& spawn: map.getMonsters()) {
        auto it = std::find_if(catalog.begin(), catalog.end(),
                               [&spawn](const MonsterCatalogEntry& candidate) {
                                   return candidate.type == spawn.type;
                               });
        const MonsterCatalogEntry* entry = (it != catalog.end()) ? &(*it) : nullptr;
        if (!entry)
            continue;
        Position screen = camera.cellToScreen(spawn.x, spawn.y);
        if (screen.x + ts <= 0 || screen.x >= CANVAS_WIDTH || screen.y + ts <= 0 ||
            screen.y >= CANVAS_HEIGHT) {
            continue;
        }
        drawMonsterFromCatalog(*entry, screen.x, screen.y, ts);
    }
    renderer.SetClipRect();
}

void Editor::renderCitizens() {
    const auto& catalog = getCitizenCatalog();
    const int ts = camera.getTileSize();
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    for (const CitizenSpawn& spawn: map.getCitizens()) {
        auto it = std::find_if(catalog.begin(), catalog.end(),
                               [&spawn](const CitizenCatalogEntry& candidate) {
                                   return candidate.type == spawn.type;
                               });
        const CitizenCatalogEntry* entry = (it != catalog.end()) ? &(*it) : nullptr;
        if (!entry)
            continue;
        Position screen = camera.cellToScreen(spawn.x, spawn.y);
        if (screen.x + ts <= 0 || screen.x >= CANVAS_WIDTH || screen.y + ts <= 0 ||
            screen.y >= CANVAS_HEIGHT) {
            continue;
        }
        drawCitizenFromCatalog(*entry, screen.x, screen.y, ts);
    }
    renderer.SetClipRect();
}

void Editor::renderSpawn() {
    const int ts = camera.getTileSize();
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    Position spawn = map.getSpawn();
    Position screen = camera.cellToScreen(spawn.x, spawn.y);
    const int charH = ts * 3 / 2;
    drawCharacter(screen.x, screen.y + ts - charH, ts, charH);

    SDL2pp::Texture& headTex = textures.get(HEAD_SHEET_PATH);
    SDL2pp::Rect headSrc(HEAD_FRAME_X, HEAD_FRAME_Y, HEAD_FRAME_W, HEAD_FRAME_H);
    const int headW = HEAD_DRAW_W * ts / TILE_NATIVE;
    const int headH = HEAD_DRAW_H * ts / TILE_NATIVE;
    const int headOverlap = HEAD_OVERLAP * ts / TILE_NATIVE;
    const int headX = screen.x + ts / 2 - headW / 2;
    const int headY = screen.y + ts - charH + headOverlap - headH;
    renderer.Copy(headTex, headSrc, SDL2pp::Rect(headX, headY, headW, headH));

    renderer.SetDrawColor(255, 235, 0, 255);
    const int cx = screen.x + ts / 2 - MARKER_SHIFT_X;
    const int cy = screen.y + ts - 4;
    for (int t = -1; t <= 1; ++t) {
        const int rx = ts / 2 - 2 + t;
        const int ry = ts / 5 + t;
        for (int i = 0; i < MARKER_SEGMENTS; ++i) {
            const double a0 = TAU * i / MARKER_SEGMENTS;
            const double a1 = TAU * (i + 1) / MARKER_SEGMENTS;
            renderer.DrawLine(cx + static_cast<int>(rx * std::cos(a0)),
                              cy + static_cast<int>(ry * std::sin(a0)),
                              cx + static_cast<int>(rx * std::cos(a1)),
                              cy + static_cast<int>(ry * std::sin(a1)));
        }
    }
    renderer.SetClipRect();
}

void Editor::drawSaveIcon(const Toolbar::Button& b) {
    int s = b.h - 12;
    int ix = b.x + 10;
    int iy = b.y + 6;
    renderer.SetDrawColor(240, 240, 240, 255);
    renderer.FillRect(SDL2pp::Rect(ix, iy, s, s));
    renderer.SetDrawColor(60, 150, 70, 255);
    renderer.FillRect(SDL2pp::Rect(ix + 4, iy, s - 10, s / 3));
    renderer.SetDrawColor(120, 120, 130, 255);
    renderer.FillRect(SDL2pp::Rect(ix + 5, iy + s / 2, s - 10, s / 3 + 1));
}

void Editor::drawEraserIcon(const Toolbar::Button& b) {
    int s = b.h - 12;
    int ix = b.x + 10;
    int iy = b.y + 6;
    renderer.SetDrawColor(240, 200, 200, 255);
    renderer.FillRect(SDL2pp::Rect(ix, iy, s, s));
    renderer.SetDrawColor(200, 100, 100, 255);
    renderer.FillRect(SDL2pp::Rect(ix, iy, s, s / 3));
    renderer.SetDrawColor(120, 60, 60, 255);
    renderer.DrawRect(SDL2pp::Rect(ix, iy, s, s));
}

void Editor::drawMapsIcon(const Toolbar::Button& b) {
    int s = b.h - 12;
    int ix = b.x + 10;
    int iy = b.y + 6;
    renderer.SetDrawColor(225, 215, 170, 255);
    renderer.FillRect(SDL2pp::Rect(ix, iy, s, s));
    renderer.SetDrawColor(120, 110, 80, 255);
    renderer.DrawLine(ix + s / 3, iy, ix + s / 3, iy + s);
    renderer.DrawLine(ix + 2 * s / 3, iy, ix + 2 * s / 3, iy + s);
    renderer.DrawRect(SDL2pp::Rect(ix, iy, s, s));
}

void Editor::drawDungeonIcon(const Toolbar::Button& b) {
    int s = b.h - 12;
    int ix = b.x + 10;
    int iy = b.y + 6;
    renderer.SetDrawColor(40, 30, 30, 255);
    renderer.FillRect(SDL2pp::Rect(ix, iy, s, s));
    renderer.SetDrawColor(220, 80, 30, 255);
    renderer.DrawRect(SDL2pp::Rect(ix, iy, s, s));
    renderer.DrawRect(SDL2pp::Rect(ix + 2, iy + 2, s - 4, s - 4));
}

void Editor::renderPanel() {
    const std::vector<OverlayDef>& overlayReg = getOverlayRegistry();
    const auto& monsterCat = getMonsterCatalog();
    const auto& citizenCat = getCitizenCatalog();

    renderer.SetDrawColor(50, 50, 60, 255);
    renderer.FillRect(SDL2pp::Rect(PANEL_X, 0, PANEL_WIDTH, WINDOW_HEIGHT));

    for (const Toolbar::Button& b: toolbar.getButtons()) {
        if (b.action == ToolbarAction::SAVE) {
            if (SDL_GetTicks() < savedFlashUntil) {
                renderer.SetDrawColor(120, 240, 130, 255);
            } else {
                renderer.SetDrawColor(60, 150, 70, 255);
            }
        } else {
            renderer.SetDrawColor(80, 80, 95, 255);
        }
        renderer.FillRect(SDL2pp::Rect(b.x, b.y, b.w, b.h));

        switch (b.action) {
            case ToolbarAction::SAVE:
                drawSaveIcon(b);
                font.drawString("Guardar", b.x + b.h + 4, b.y + (b.h - LABEL_FONT_SIZE) / 2 - 2,
                                LABEL_COLOR);
                break;
            case ToolbarAction::TOOL_CHANGED:
                font.drawString(labelForTool(b.tool), b.x + b.h + 4,
                                b.y + (b.h - LABEL_FONT_SIZE) / 2 - 2, LABEL_COLOR);
                switch (b.tool) {
                    case Tool::OVERLAY:
                        drawGrass(b.x + 4, b.y + 2, b.h - 4);
                        drawOverlay(overlayReg[itemOverlays[itemPalette.getSelectedTile()]],
                                    b.x + 4, b.y + 2, b.h - 4);
                        break;
                    case Tool::MONSTER:
                        drawGrass(b.x + 4, b.y + 2, b.h - 4);
                        drawMonsterFromCatalog(monsterCat[monsterPalette.getSelectedTile()],
                                               b.x + 4, b.y + 2, b.h - 4);
                        break;
                    case Tool::CITIZEN:
                        drawGrass(b.x + 4, b.y + 2, b.h - 4);
                        drawCitizenFromCatalog(citizenCat[citizenPalette.getSelectedTile()],
                                               b.x + 4, b.y + 2, b.h - 4);
                        break;
                    case Tool::CITY:
                        drawGrass(b.x + 4, b.y + 2, b.h - 4);
                        break;
                    case Tool::CITY_ERASE:
                    case Tool::ERASER:
                        drawEraserIcon(b);
                        break;
                    case Tool::DUNGEON:
                        drawDungeonIcon(b);
                        break;
                    case Tool::DUNGEON_ERASE:
                        drawEraserIcon(b);
                        break;
                    case Tool::SPAWN:
                        drawCharacter(b.x + 4, b.y + 2, b.h - 4, b.h - 4);
                        break;
                }
                break;
            case ToolbarAction::OPEN_MAPS:
                drawMapsIcon(b);
                font.drawString("Mapas", b.x + b.h + 4, b.y + (b.h - LABEL_FONT_SIZE) / 2 - 2,
                                LABEL_COLOR);
                break;
            case ToolbarAction::NONE:
                break;
        }

        if (b.action == ToolbarAction::TOOL_CHANGED && b.tool == toolbar.getActiveTool()) {
            renderer.SetDrawColor(255, 235, 0, 255);
            renderer.DrawRect(SDL2pp::Rect(b.x, b.y, b.w, b.h));
        }
    }

    const Palette& pal = activePalette();
    Tool tool = toolbar.getActiveTool();
    int count = pal.getTileCount();
    for (int i = 0; i < count; ++i) {
        int col = i % pal.getCols();
        int row = i / pal.getCols() - pal.getScrollRow();
        if (row < 0) {
            continue;
        }
        int dx = pal.getPanelX() + col * pal.getTileDrawSize();
        int dy = pal.getPanelY() + row * pal.getTileDrawSize();
        if (dy + pal.getTileDrawSize() > CANVAS_HEIGHT) {
            break;
        }
        drawGrass(dx, dy, pal.getTileDrawSize());
        switch (tool) {
            case Tool::MONSTER:
                drawMonsterFromCatalog(monsterCat[i], dx, dy, pal.getTileDrawSize());
                break;
            case Tool::CITIZEN:
                drawCitizenFromCatalog(citizenCat[i], dx, dy, pal.getTileDrawSize());
                break;
            case Tool::OVERLAY:
            case Tool::CITY:
            case Tool::CITY_ERASE:
            case Tool::ERASER:
            case Tool::SPAWN:
            default:
                drawOverlay(overlayReg[itemOverlays[i]], dx, dy, pal.getTileDrawSize());
                break;
        }
        if (i == pal.getSelectedTile()) {
            renderer.SetDrawColor(255, 235, 0, 255);
            renderer.DrawRect(SDL2pp::Rect(dx, dy, pal.getTileDrawSize(), pal.getTileDrawSize()));
        }
    }
}

void Editor::renderStatusBar() {
    const std::vector<OverlayDef>& overlayReg = getOverlayRegistry();
    const auto& monsterCat = getMonsterCatalog();
    const auto& citizenCat = getCitizenCatalog();

    renderer.SetDrawColor(20, 20, 25, 255);
    renderer.FillRect(SDL2pp::Rect(0, CANVAS_HEIGHT, WINDOW_WIDTH, STATUS_HEIGHT));

    int iconSize = STATUS_HEIGHT - 8;
    int x0 = 6;
    int y0 = CANVAS_HEIGHT + 4;
    Tool active = toolbar.getActiveTool();
    std::string selectedName;
    switch (active) {
        case Tool::SPAWN:
            drawCharacter(x0, y0, iconSize, iconSize);
            break;
        case Tool::ERASER: {
            SDL2pp::Rect r(x0, y0, iconSize, iconSize);
            renderer.SetDrawColor(240, 200, 200, 255);
            renderer.FillRect(r);
            renderer.SetDrawColor(200, 100, 100, 255);
            renderer.DrawRect(r);
            break;
        }
        case Tool::MONSTER:
            drawGrass(x0, y0, iconSize);
            drawMonsterFromCatalog(monsterCat[monsterPalette.getSelectedTile()], x0, y0, iconSize);
            selectedName = monsterCat[monsterPalette.getSelectedTile()].type;
            break;
        case Tool::CITIZEN:
            drawGrass(x0, y0, iconSize);
            drawCitizenFromCatalog(citizenCat[citizenPalette.getSelectedTile()], x0, y0, iconSize);
            selectedName = citizenCat[citizenPalette.getSelectedTile()].type;
            break;
        case Tool::CITY:
        case Tool::CITY_ERASE:
        case Tool::DUNGEON:
        case Tool::DUNGEON_ERASE:
            drawGrass(x0, y0, iconSize);
            break;
        case Tool::OVERLAY:
        default:
            drawGrass(x0, y0, iconSize);
            drawOverlay(overlayReg[itemOverlays[itemPalette.getSelectedTile()]], x0, y0, iconSize);
            selectedName = overlayReg[itemOverlays[itemPalette.getSelectedTile()]].name;
            break;
    }
    std::string status = labelForTool(active);
    if (!selectedName.empty()) {
        status += ": " + selectedName;
    }
    if (!statusMsg.empty()) {
        status += "  |  " + statusMsg;
    }
    font.drawString(status, x0 + iconSize + 8, y0 + (iconSize - LABEL_FONT_SIZE) / 2 - 2,
                    LABEL_COLOR);
}
