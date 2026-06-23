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

inline TileRect bottomBandRect(const TileRect& full, int bandTiles, int tileSize) {
    int bandH = bandTiles * tileSize;
    if (bandH > full.h) {
        bandH = full.h;
    }
    return TileRect{full.x, full.y + (full.h - bandH), full.w, bandH};
}

#endif
