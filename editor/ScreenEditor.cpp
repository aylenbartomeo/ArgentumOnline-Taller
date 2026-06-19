#include "ScreenEditor.h"

#include <filesystem>

#include "loop/ConstantRateLoop.h"

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
        rightDragging(false),
        lastMouseX(0),
        lastMouseY(0),
        currentMapPath(MapDefaults::DEFAULT_MAP_PATH),
        savedFlashUntil(0) {
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
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            SDL2pp::Point p = toMockup(event.button.x, event.button.y);
            Region region = regionAtClick(screen, p.x, p.y);
            if (region == Region::GO_TERRENO || region == Region::GO_ITEMS ||
                region == Region::GO_MONSTRUOS || region == Region::GO_CIUDADANOS) {
                screen = screenForRegion(region);
            } else if (region == Region::BACK) {
                screen = Screen::PRINCIPAL;
            } else if (region == Region::GOMA) {
                activeTool = Tool::ERASER;
            } else if (region == Region::SPAWN) {
                activeTool = Tool::SPAWN;
            } else if (region == Region::GUARDAR) {
                saveMap();
            } else if (region == Region::CANVAS && activeTool != Tool::NONE) {
                LayoutRect c = canvasRect();
                Position cell = camera.screenToCell(p.x - c.x, p.y - c.y);
                if (activeTool == Tool::ERASER) {
                    smartEraseAt(map, cell.x, cell.y);
                } else if (activeTool == Tool::SPAWN) {
                    map.setSpawn(cell.x, cell.y);
                }
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
        renderer.SetDrawColor(255, 235, 0, 255);
        for (int i = 0; i < 4; ++i) {
            renderer.DrawRect(SDL2pp::Rect(g.x + i, g.y + i, g.w - 2 * i, g.h - 2 * i));
        }
    }
    if (screen == Screen::TERRENO) {
        LayoutRect b = terrenoBackRect();
        SDL2pp::Texture& back = textures.get(std::string(EDITOR_RES) + "BackTerreno.png");
        renderer.Copy(back, SDL2pp::NullOpt, SDL2pp::Rect(b.x, b.y, b.w, b.h));
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
