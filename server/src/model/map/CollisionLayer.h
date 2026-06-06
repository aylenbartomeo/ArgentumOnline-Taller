#ifndef COLLISION_LAYER_H
#define COLLISION_LAYER_H

#include <cstdint>
#include <vector>

#include "position.h"

class CollisionLayer {
private:
    int width;
    int height;
    std::vector<std::vector<bool>> grid;

public:
    CollisionLayer();
    CollisionLayer(int w, int h);
    ~CollisionLayer() = default;

    void resize(int w, int h);
    void clear();

    void setSolid(int x, int y, bool is_solid);
    bool isSolid(int x, int y) const;

    // Retorna true si hay colisión, evaluando con un offset (hitbox)
    bool checkPlayerCollision(float pos_x, float pos_y) const;

    // Retorna true si existe línea de visión (sin obstáculos sólidos) entre dos posiciones
    bool hasLineOfSight(const Position& from, const Position& to) const;
};

#endif  // COLLISION_LAYER_H
