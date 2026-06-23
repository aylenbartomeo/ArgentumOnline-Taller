#include "EditorLayout.h"

namespace {
constexpr LayoutRect CANVAS = {64, 72, 1018, 960};
constexpr LayoutRect GOMA = {48, 48, 152, 156};
constexpr LayoutRect SPAWN = {200, 48, 135, 156};
constexpr LayoutRect GUARDAR = {1118, 148, 125, 95};
constexpr LayoutRect MAPAS = {1284, 80, 85, 120};
constexpr LayoutRect BACK_TERRENO = {35, 900, 165, 150};
constexpr LayoutRect BACK_PANEL = {1098, 958, 95, 90};
constexpr LayoutRect NAV_CIUDADANOS = {1130, 300, 290, 130};
constexpr LayoutRect NAV_ITEMS = {1130, 478, 290, 117};
constexpr LayoutRect NAV_MONSTRUOS = {1130, 638, 290, 117};
constexpr LayoutRect NAV_TERRENO = {1130, 808, 290, 117};
constexpr LayoutRect TOOLS = {35, 50, 300, 187};
constexpr int BLOCK_X = 1160;
constexpr int BLOCK_W = 195;
constexpr int BLOCK_H = 120;
constexpr int BLOCK_Y0 = 308;
constexpr int BLOCK_STEP_Y = 150;
constexpr LayoutRect BLOCK_PLAYA = {BLOCK_X, BLOCK_Y0, BLOCK_W, BLOCK_H};
constexpr LayoutRect BLOCK_BOSQUE = {BLOCK_X, BLOCK_Y0 + BLOCK_STEP_Y, BLOCK_W, BLOCK_H};
constexpr LayoutRect BLOCK_DESIERTO = {BLOCK_X, BLOCK_Y0 + 2 * BLOCK_STEP_Y, BLOCK_W, BLOCK_H};
constexpr LayoutRect BLOCK_CIUDAD = {BLOCK_X, BLOCK_Y0 + 3 * BLOCK_STEP_Y, BLOCK_W, BLOCK_H};
constexpr LayoutRect BLOCK_MAZMORRA = {BLOCK_X, BLOCK_Y0 + 4 * BLOCK_STEP_Y, BLOCK_W, BLOCK_H};

bool inside(LayoutRect r, int x, int y) {
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}
}  // namespace

LayoutRect canvasRect() { return CANVAS; }

Region regionAtClick(Screen screen, int mx, int my) {
    if (inside(GOMA, mx, my)) {
        return Region::GOMA;
    }
    if (inside(SPAWN, mx, my)) {
        return Region::SPAWN;
    }
    if (inside(GUARDAR, mx, my)) {
        return Region::GUARDAR;
    }
    if (inside(MAPAS, mx, my)) {
        return Region::MAPAS;
    }
    if (screen == Screen::PRINCIPAL) {
        if (inside(NAV_TERRENO, mx, my)) {
            return Region::GO_TERRENO;
        }
        if (inside(NAV_ITEMS, mx, my)) {
            return Region::GO_ITEMS;
        }
        if (inside(NAV_MONSTRUOS, mx, my)) {
            return Region::GO_MONSTRUOS;
        }
        if (inside(NAV_CIUDADANOS, mx, my)) {
            return Region::GO_CIUDADANOS;
        }
    } else if (screen == Screen::TERRENO) {
        if (inside(BACK_TERRENO, mx, my)) {
            return Region::BACK;
        }
        if (inside(BLOCK_PLAYA, mx, my)) {
            return Region::BLOCK_PLAYA;
        }
        if (inside(BLOCK_BOSQUE, mx, my)) {
            return Region::BLOCK_BOSQUE;
        }
        if (inside(BLOCK_DESIERTO, mx, my)) {
            return Region::BLOCK_DESIERTO;
        }
        if (inside(BLOCK_CIUDAD, mx, my)) {
            return Region::BLOCK_CIUDAD;
        }
        if (inside(BLOCK_MAZMORRA, mx, my)) {
            return Region::BLOCK_MAZMORRA;
        }
    } else if (inside(BACK_PANEL, mx, my)) {
        return Region::BACK;
    }
    if (inside(CANVAS, mx, my)) {
        return Region::CANVAS;
    }
    return Region::NONE;
}

std::string mockupFile(Screen screen) {
    switch (screen) {
        case Screen::PRINCIPAL:
            return "EditorPrincipal.png";
        case Screen::TERRENO:
            return "EditorTerrenos.png";
        case Screen::ITEMS:
            return "EditorItems.png";
        case Screen::MONSTRUOS:
            return "EditorMonstruos.png";
        case Screen::CIUDADANOS:
            return "EditorCiudadanos.png";
    }
    return "EditorPrincipal.png";
}

Screen screenForRegion(Region region) {
    switch (region) {
        case Region::GO_TERRENO:
            return Screen::TERRENO;
        case Region::GO_ITEMS:
            return Screen::ITEMS;
        case Region::GO_MONSTRUOS:
            return Screen::MONSTRUOS;
        case Region::GO_CIUDADANOS:
            return Screen::CIUDADANOS;
        default:
            return Screen::PRINCIPAL;
    }
}

LayoutRect topLeftToolsRect() { return TOOLS; }

LayoutRect terrenoBackRect() { return BACK_TERRENO; }

LayoutRect gomaRect() { return GOMA; }

LayoutRect spawnRect() { return SPAWN; }

LayoutRect guardarRect() { return GUARDAR; }

LayoutRect mapasRect() { return MAPAS; }

LayoutRect blockRect(Region region) {
    switch (region) {
        case Region::BLOCK_PLAYA:
            return BLOCK_PLAYA;
        case Region::BLOCK_BOSQUE:
            return BLOCK_BOSQUE;
        case Region::BLOCK_DESIERTO:
            return BLOCK_DESIERTO;
        case Region::BLOCK_CIUDAD:
            return BLOCK_CIUDAD;
        case Region::BLOCK_MAZMORRA:
            return BLOCK_MAZMORRA;
        default:
            return {0, 0, 0, 0};
    }
}
