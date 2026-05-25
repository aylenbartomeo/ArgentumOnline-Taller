#ifndef POSITION_H
#define POSITION_H

#include <algorithm>
#include <cmath>

// Representa una coordenada en el mapa basada en tiles.
struct Position {
    int x;
    int y;

    bool operator==(const Position& other) const { return x == other.x && y == other.y; }

    bool operator!=(const Position& other) const { return !(*this == other); }

    // Método de utilidad para calcular distancia de Manhattan
    int distance_to(const Position& other) const { return abs(x - other.x) + abs(y - other.y); }

    // Distancia Chebyshev: movimientos en un grid 8-direccional
    int chebyshev_distance_to(const Position& other) const {
        return std::max(std::abs(x - other.x), std::abs(y - other.y));
    }

    // Podríamos agregar un método para verificar si una posición
    // es adyacente (útil para interacciones como abrir cofres o hablar)
    bool is_adjacent(const Position& other) const { return distance_to(other) == 1; }

    // Verifica que la posición esté dentro de los límites [0, width) x [0, height)
    bool isWithinBounds(int width, int height) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }
};

#endif
