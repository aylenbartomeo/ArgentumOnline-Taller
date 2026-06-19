#include "EditorLayout.h"

namespace {
constexpr LayoutRect CANVAS = {130, 100, 950, 885};
constexpr LayoutRect GOMA = {48, 48, 152, 156};
constexpr LayoutRect GUARDAR = {1130, 25, 110, 95};
constexpr LayoutRect MAPAS = {1250, 25, 125, 95};
constexpr LayoutRect BACK = {28, 895, 105, 95};
constexpr LayoutRect NAV_CIUDADANOS = {1140, 195, 270, 95};
constexpr LayoutRect NAV_ITEMS = {1140, 365, 270, 95};
constexpr LayoutRect NAV_MONSTRUOS = {1140, 535, 270, 95};
constexpr LayoutRect NAV_TERRENO = {1140, 705, 270, 95};

bool inside(LayoutRect r, int x, int y) {
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}
}  // namespace

LayoutRect canvasRect() { return CANVAS; }

Region regionAtClick(Screen screen, int mx, int my) {
    if (inside(GOMA, mx, my)) {
        return Region::GOMA;
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
    } else if (inside(BACK, mx, my)) {
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
            return "EditorTerreno.png";
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
