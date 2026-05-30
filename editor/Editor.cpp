#include "Editor.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

namespace {
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr int PANEL_WIDTH = 200;
constexpr int STATUS_HEIGHT = 40;
constexpr int CANVAS_WIDTH = WINDOW_WIDTH - PANEL_WIDTH;
constexpr int CANVAS_HEIGHT = WINDOW_HEIGHT - STATUS_HEIGHT;
constexpr int TILE_SCREEN = 32;
constexpr int PANEL_X = CANVAS_WIDTH;
constexpr int CAMERA_STEP = 32;

constexpr int BTN_W = PANEL_WIDTH - 20;
constexpr int BTN_H = 34;
constexpr int PINCEL_X = PANEL_X + 10;
constexpr int PINCEL_Y = 10;
constexpr int ERASER_BTN_Y = 50;
constexpr int SPAWN_BTN_Y = 90;

constexpr int SIZE_HINT_X = PANEL_X + 12;
constexpr int SIZE_BTN_W = 32;
constexpr int SIZE_BTN_H = 28;
constexpr int WIDTH_ROW_Y = 136;
constexpr int HEIGHT_ROW_Y = 170;
constexpr int SIZE_MINUS_X = PANEL_X + 66;
constexpr int SIZE_PLUS_X = PANEL_X + 104;

constexpr int SAVE_Y = 206;

constexpr int PALETTE_X = PANEL_X + 10;
constexpr int PALETTE_Y = 250;
constexpr int PALETTE_TILE = 32;
constexpr int PALETTE_COLS = 5;

constexpr const char* RESOURCES_DIR = "resources/";
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
}  // namespace

Editor::Editor(EditorMap initialMap, const std::string& mapPath):
        sdl(SDL_INIT_VIDEO),
        window("Editor de mapas - Argentum Online", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
               WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN),
        renderer(window, -1, SDL_RENDERER_ACCELERATED),
        textures(renderer),
        map(std::move(initialMap)),
        camera(CANVAS_WIDTH, CANVAS_HEIGHT, TILE_SCREEN, map.getWidth(), map.getHeight()),
        palette(PALETTE_X, PALETTE_Y, PALETTE_TILE, PALETTE_COLS,
                static_cast<int>(getOverlayRegistry().size())),
        toolbar(),
        mapPath(mapPath),
        rightDragging(false),
        lastMouseX(0),
        lastMouseY(0),
        dirty(false),
        savedFlashUntil(0) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    toolbar.addToolButton(PINCEL_X, PINCEL_Y, BTN_W, BTN_H, Tool::PINCEL);
    toolbar.addToolButton(PINCEL_X, ERASER_BTN_Y, BTN_W, BTN_H, Tool::ERASER);
    toolbar.addToolButton(PINCEL_X, SPAWN_BTN_Y, BTN_W, BTN_H, Tool::SPAWN);
    toolbar.addActionButton(SIZE_MINUS_X, WIDTH_ROW_Y, SIZE_BTN_W, SIZE_BTN_H,
                            ToolbarAction::WIDTH_MINUS);
    toolbar.addActionButton(SIZE_PLUS_X, WIDTH_ROW_Y, SIZE_BTN_W, SIZE_BTN_H,
                            ToolbarAction::WIDTH_PLUS);
    toolbar.addActionButton(SIZE_MINUS_X, HEIGHT_ROW_Y, SIZE_BTN_W, SIZE_BTN_H,
                            ToolbarAction::HEIGHT_MINUS);
    toolbar.addActionButton(SIZE_PLUS_X, HEIGHT_ROW_Y, SIZE_BTN_W, SIZE_BTN_H,
                            ToolbarAction::HEIGHT_PLUS);
    toolbar.addActionButton(PINCEL_X, SAVE_Y, BTN_W, BTN_H, ToolbarAction::SAVE);

    updateTitle();
}

void Editor::run() {
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleEvent(event, running);
        }
        render();
        SDL_Delay(16);
    }
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
    } else if (event.type == SDL_KEYDOWN) {
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
            case SDLK_s:
                save();
                break;
            default:
                break;
        }
    }
}

void Editor::handleLeftClick(int x, int y) {
    if (x < CANVAS_WIDTH && y < CANVAS_HEIGHT) {
        Position cell = camera.screenToCell(x, y);
        if (cell.x >= 0 && cell.x < map.getWidth() && cell.y >= 0 && cell.y < map.getHeight()) {
            switch (toolbar.getActiveTool()) {
                case Tool::PINCEL:
                    map.setTile(cell.x, cell.y, palette.getSelectedTile() + 1);
                    break;
                case Tool::ERASER:
                    map.setTile(cell.x, cell.y, 0);
                    break;
                case Tool::SPAWN:
                    map.setSpawn(cell.x, cell.y);
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
        case ToolbarAction::WIDTH_PLUS:
            map.resize(map.getWidth() + 1, map.getHeight());
            camera.setMapSize(map.getWidth(), map.getHeight());
            dirty = true;
            updateTitle();
            break;
        case ToolbarAction::WIDTH_MINUS:
            if (map.getWidth() > 1) {
                map.resize(map.getWidth() - 1, map.getHeight());
                camera.setMapSize(map.getWidth(), map.getHeight());
                dirty = true;
                updateTitle();
            }
            break;
        case ToolbarAction::HEIGHT_PLUS:
            map.resize(map.getWidth(), map.getHeight() + 1);
            camera.setMapSize(map.getWidth(), map.getHeight());
            dirty = true;
            updateTitle();
            break;
        case ToolbarAction::HEIGHT_MINUS:
            if (map.getHeight() > 1) {
                map.resize(map.getWidth(), map.getHeight() - 1);
                camera.setMapSize(map.getWidth(), map.getHeight());
                dirty = true;
                updateTitle();
            }
            break;
        case ToolbarAction::TOOL_CHANGED:
            break;
        case ToolbarAction::NONE:
            palette.selectFromClick(x, y);
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

void Editor::drawGrass(int dstX, int dstY, int dstSize) {
    SDL2pp::Texture& tex = textures.get(GRASS_SHEET_PATH);
    const SDL2pp::Rect srcRect(GRASS_SRC_X, GRASS_SRC_Y, GRASS_SRC_SIZE, GRASS_SRC_SIZE);
    const SDL2pp::Rect dstRect(dstX, dstY, dstSize, dstSize);
    renderer.Copy(tex, srcRect, dstRect);
}

void Editor::drawOverlay(const OverlayDef& def, int cellX, int cellY, int cellSize) {
    SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + def.tilesheet);
    const SDL2pp::Rect srcRect(def.srcX, def.srcY, def.srcW, def.srcH);
    const int dstW = cellSize;
    const int dstH = (def.srcH * cellSize) / def.srcW;
    const int dstX = cellX;
    const int dstY = cellY + cellSize - dstH;
    renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));
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
    renderOverlays();
    renderSpawn();
    renderPanel();
    renderStatusBar();
    renderer.Present();
}

void Editor::renderTerrain() {
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            Position screen = camera.cellToScreen(col, row);
            if (screen.x + TILE_SCREEN <= 0 || screen.x >= CANVAS_WIDTH ||
                screen.y + TILE_SCREEN <= 0 || screen.y >= CANVAS_HEIGHT) {
                continue;
            }
            drawGrass(screen.x, screen.y, TILE_SCREEN);
        }
    }
    renderer.SetClipRect();
}

void Editor::renderOverlays() {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int tileId = map.tileAt(col, row);
            if (tileId <= 0 || tileId > static_cast<int>(registry.size())) {
                continue;
            }
            Position screen = camera.cellToScreen(col, row);
            if (screen.x + TILE_SCREEN <= 0 || screen.x >= CANVAS_WIDTH ||
                screen.y + TILE_SCREEN <= 0 || screen.y >= CANVAS_HEIGHT) {
                continue;
            }
            drawOverlay(registry[tileId - 1], screen.x, screen.y, TILE_SCREEN);
        }
    }
    renderer.SetClipRect();
}

void Editor::renderSpawn() {
    renderer.SetClipRect(SDL2pp::Rect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
    Position spawn = map.getSpawn();
    Position screen = camera.cellToScreen(spawn.x, spawn.y);
    const int charH = TILE_SCREEN * 3 / 2;
    drawCharacter(screen.x, screen.y + TILE_SCREEN - charH, TILE_SCREEN, charH);

    SDL2pp::Texture& headTex = textures.get(HEAD_SHEET_PATH);
    SDL2pp::Rect headSrc(HEAD_FRAME_X, HEAD_FRAME_Y, HEAD_FRAME_W, HEAD_FRAME_H);
    const int headX = screen.x + TILE_SCREEN / 2 - HEAD_DRAW_W / 2;
    const int headY = screen.y + TILE_SCREEN - charH + HEAD_OVERLAP - HEAD_DRAW_H;
    renderer.Copy(headTex, headSrc, SDL2pp::Rect(headX, headY, HEAD_DRAW_W, HEAD_DRAW_H));

    renderer.SetDrawColor(255, 235, 0, 255);
    const int cx = screen.x + TILE_SCREEN / 2 - MARKER_SHIFT_X;
    const int cy = screen.y + TILE_SCREEN - 4;
    for (int t = -1; t <= 1; ++t) {
        const int rx = TILE_SCREEN / 2 - 2 + t;
        const int ry = TILE_SCREEN / 5 + t;
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

void Editor::drawMinus(const Toolbar::Button& b) {
    renderer.SetDrawColor(240, 240, 240, 255);
    int barW = b.w / 2;
    int barH = 4;
    renderer.FillRect(SDL2pp::Rect(b.x + (b.w - barW) / 2, b.y + b.h / 2 - barH / 2, barW, barH));
}

void Editor::drawPlus(const Toolbar::Button& b) {
    renderer.SetDrawColor(240, 240, 240, 255);
    int barW = b.w / 2;
    int barH = 4;
    renderer.FillRect(SDL2pp::Rect(b.x + (b.w - barW) / 2, b.y + b.h / 2 - barH / 2, barW, barH));
    int vBarH = b.h / 2;
    renderer.FillRect(SDL2pp::Rect(b.x + b.w / 2 - barH / 2, b.y + (b.h - vBarH) / 2, barH, vBarH));
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

void Editor::renderPanel() {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();

    renderer.SetDrawColor(50, 50, 60, 255);
    renderer.FillRect(SDL2pp::Rect(PANEL_X, 0, PANEL_WIDTH, WINDOW_HEIGHT));

    renderer.SetDrawColor(200, 200, 210, 255);
    renderer.FillRect(SDL2pp::Rect(SIZE_HINT_X, WIDTH_ROW_Y + SIZE_BTN_H / 2 - 2, 40, 4));
    renderer.FillRect(SDL2pp::Rect(SIZE_HINT_X + 18, HEIGHT_ROW_Y, 4, SIZE_BTN_H));

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
            case ToolbarAction::WIDTH_MINUS:
            case ToolbarAction::HEIGHT_MINUS:
                drawMinus(b);
                break;
            case ToolbarAction::WIDTH_PLUS:
            case ToolbarAction::HEIGHT_PLUS:
                drawPlus(b);
                break;
            case ToolbarAction::SAVE:
                drawSaveIcon(b);
                break;
            case ToolbarAction::TOOL_CHANGED:
                if (b.tool == Tool::SPAWN) {
                    drawCharacter(b.x + 4, b.y + 2, b.h - 4, b.h - 4);
                } else if (b.tool == Tool::ERASER) {
                    drawEraserIcon(b);
                } else {
                    drawGrass(b.x + 4, b.y + 2, b.h - 4);
                    drawOverlay(registry[palette.getSelectedTile()], b.x + 4, b.y + 2, b.h - 4);
                }
                break;
            case ToolbarAction::NONE:
                break;
        }

        if (b.action == ToolbarAction::TOOL_CHANGED && b.tool == toolbar.getActiveTool()) {
            renderer.SetDrawColor(255, 235, 0, 255);
            renderer.DrawRect(SDL2pp::Rect(b.x, b.y, b.w, b.h));
        }
    }

    int count = palette.getTileCount();
    for (int i = 0; i < count; ++i) {
        int col = i % palette.getCols();
        int row = i / palette.getCols();
        int dx = palette.getPanelX() + col * palette.getTileDrawSize();
        int dy = palette.getPanelY() + row * palette.getTileDrawSize();
        if (dy + palette.getTileDrawSize() > CANVAS_HEIGHT) {
            break;
        }
        drawGrass(dx, dy, palette.getTileDrawSize());
        drawOverlay(registry[i], dx, dy, palette.getTileDrawSize());
        if (i == palette.getSelectedTile()) {
            renderer.SetDrawColor(255, 235, 0, 255);
            renderer.DrawRect(
                    SDL2pp::Rect(dx, dy, palette.getTileDrawSize(), palette.getTileDrawSize()));
        }
    }
}

void Editor::renderStatusBar() {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();

    renderer.SetDrawColor(20, 20, 25, 255);
    renderer.FillRect(SDL2pp::Rect(0, CANVAS_HEIGHT, WINDOW_WIDTH, STATUS_HEIGHT));

    int iconSize = STATUS_HEIGHT - 8;
    Tool active = toolbar.getActiveTool();
    if (active == Tool::SPAWN) {
        drawCharacter(6, CANVAS_HEIGHT + 4, iconSize, iconSize);
    } else if (active == Tool::ERASER) {
        SDL2pp::Rect r(6, CANVAS_HEIGHT + 4, iconSize, iconSize);
        renderer.SetDrawColor(240, 200, 200, 255);
        renderer.FillRect(r);
        renderer.SetDrawColor(200, 100, 100, 255);
        renderer.DrawRect(r);
    } else {
        drawGrass(6, CANVAS_HEIGHT + 4, iconSize);
        drawOverlay(registry[palette.getSelectedTile()], 6, CANVAS_HEIGHT + 4, iconSize);
    }
    drawGrass(6 + iconSize + 8, CANVAS_HEIGHT + 4, iconSize);
    drawOverlay(registry[palette.getSelectedTile()], 6 + iconSize + 8, CANVAS_HEIGHT + 4, iconSize);
}
