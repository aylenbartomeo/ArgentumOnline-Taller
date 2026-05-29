#include "CollisionLayer.h"

#include <cmath>

CollisionLayer::CollisionLayer() : width(0), height(0) {}

CollisionLayer::CollisionLayer(int w, int h) : width(w), height(h) {
    grid.assign(w, std::vector<bool>(h, false));
}

void CollisionLayer::resize(int w, int h) {
    this->width = w;
    this->height = h;
    grid.assign(w, std::vector<bool>(h, false));
}

void CollisionLayer::clear() {
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            grid[x][y] = false;
        }
    }
}

void CollisionLayer::setSolid(int x, int y, bool is_solid) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        grid[x][y] = is_solid;
    }
}

bool CollisionLayer::isSolid(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return grid[x][y];
    }
    return true; // Fuera del mapa es considerado sólido
}

bool CollisionLayer::checkPlayerCollision(float pos_x, float pos_y) const {
    // Tomamos los cuatro extremos del cuerpo del jugador (asumiendo un tamaño de 0.6 de baldosa)
    float offset = 0.3f;

    // Convertimos las coordenadas continuas (float) a índices discretos de baldosas (int)
    int min_tile_x = static_cast<int>(std::floor(pos_x - offset));
    int max_tile_x = static_cast<int>(std::floor(pos_x + offset));
    int min_tile_y = static_cast<int>(std::floor(pos_y - offset));
    int max_tile_y = static_cast<int>(std::floor(pos_y + offset));

    // 1. Validar límites del mapa
    if (min_tile_x < 0 || max_tile_x >= width || min_tile_y < 0 || max_tile_y >= height) {
        return true;  // Colisión con el fin del mundo
    }

    // 2. Validar contra la matriz en O(1). Si cualquiera de las esquinas pisa un bloqueo, rebota.
    if (grid[min_tile_x][min_tile_y] || grid[max_tile_x][min_tile_y] ||
        grid[min_tile_x][max_tile_y] || grid[max_tile_x][max_tile_y]) {
        return true;
    }

    return false;
}

bool CollisionLayer::hasLineOfSight(const Position& from, const Position& to) const {
    int x0 = from.x;
    int y0 = from.y;
    int x1 = to.x;
    int y1 = to.y;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (isSolid(x0, y0)) {
            return false;
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
    return true;
}
