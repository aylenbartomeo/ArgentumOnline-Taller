#ifndef TILE_DRAW_H
#define TILE_DRAW_H

struct TileRect {
    int x;
    int y;
    int w;
    int h;
};

inline TileRect tileDestRect(int col, int row, int texW, int texH, int tileSize, int camX,
                             int camY) {
    return TileRect{col * tileSize - camX, (row + 1) * tileSize - texH - camY, texW, texH};
}

#endif
