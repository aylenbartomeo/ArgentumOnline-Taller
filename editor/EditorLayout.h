#ifndef EDITOR_LAYOUT_H
#define EDITOR_LAYOUT_H

#include <string>

enum class Screen { PRINCIPAL, TERRENO, ITEMS, MONSTRUOS, CIUDADANOS };

enum class Region {
    NONE,
    CANVAS,
    GO_TERRENO,
    GO_ITEMS,
    GO_MONSTRUOS,
    GO_CIUDADANOS,
    BACK,
    GOMA,
    SPAWN,
    GUARDAR,
    MAPAS,
    BLOCK_PLAYA,
    BLOCK_BOSQUE,
    BLOCK_DESIERTO,
    BLOCK_CIUDAD,
    BLOCK_MAZMORRA
};

struct LayoutRect {
    int x;
    int y;
    int w;
    int h;
};

LayoutRect canvasRect();
Region regionAtClick(Screen screen, int mx, int my);
std::string mockupFile(Screen screen);
Screen screenForRegion(Region region);
LayoutRect topLeftToolsRect();
LayoutRect terrenoBackRect();
LayoutRect gomaRect();
LayoutRect spawnRect();
LayoutRect guardarRect();
LayoutRect mapasRect();
LayoutRect blockRect(Region region);

#endif
