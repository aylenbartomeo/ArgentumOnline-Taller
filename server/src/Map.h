#ifndef MAP_H
#define MAP_H

#include "../common/utils/position.h"

/**
 * @class Map
 * @brief Pendiente de implementación. Esta clase representaría el mapa del juego, incluyendo la disposición de los objetos, NPCs, etc.
 * Responsabilidad: Almacenar la grilla del mundo y validar colisiones o movimientos.
 */
class Map {
private:
    int width, height;
public:
    explicit Map();  
    
    bool is_walkable(Position position) const; // Verifica si la posición es transitable
};

#endif
