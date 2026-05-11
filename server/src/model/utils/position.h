#ifndef POSITION_H
#define POSITION_H

#include <cmath>

// Representa una coordenada en el mapa basada en tiles.
struct Position {
    int x;
    int y;

    bool operator==(const Position& other) const { return x == other.x && y == other.y; }

    bool operator!=(const Position& other) const { return !(*this == other); }

    // Método de utilidad para calcular distancia de Manhattan
    int distance_to(const Position& other) const { return abs(x - other.x) + abs(y - other.y); }

    // Podríamos agregar un método para verificar si una posición
    // es adyacente (útil para interacciones como abrir cofres o hablar)
    bool is_adjacent(const Position& other) const { return distance_to(other) == 1; }
};

#endif
