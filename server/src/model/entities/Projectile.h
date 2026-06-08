#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <cstdint>

#include "../common/include/dto/Snapshot.h"

class Projectile {
public:
    uint32_t id;
    uint32_t ownerDbId;
    float velX, velY;    // Tiles por milisegundo
    float maxRange;      // Rango máximo en tiles antes de desaparecer
    float distTraveled;  // Distancia recorrida hasta ahora
    uint16_t spriteId;
    bool alive = true;
    Projectile(uint32_t id, uint32_t owner, float startX, float startY, float targetX,
               float targetY, float speedTilesPerSec, float maxRange, uint16_t sprite);
    // Avanza la física para un dt en milisegundos.
    // Retorna false si llegó al rango máximo.
    bool step(float dtMs);
    ProjectileDTO toDTO() const;
};

#endif
