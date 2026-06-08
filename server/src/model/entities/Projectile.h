#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <cstdint>

#include "../../../common/include/dto/Snapshot.h"

enum class ProjectileType : uint8_t {
    ARROW,        // Arcos
    MAGIC_ARROW,  // Vara de fresno
    MISSILE,      // Báculo nudoso
    EXPLOSION,    // Báculo engarzado
};

class Projectile {
public:
    uint32_t id;
    uint32_t ownerDbId;
    float x, y;          // posición actual (sub-tile precision)
    float velX, velY;    // Tiles por milisegundo
    float maxRange;      // Rango máximo en tiles antes de desaparecer
    float distTraveled;  // Distancia recorrida hasta ahora
    uint16_t spriteId;
    bool alive = true;

    int minDamage;
    int maxDamage;
    bool isMagical;
    ProjectileType type;

    Projectile(uint32_t id, uint32_t owner, float startX, float startY, float targetX,
               float targetY, float speedTilesPerSec, float maxRange, uint16_t sprite, int minDmg,
               int maxDmg, bool magical, ProjectileType type = ProjectileType::ARROW);

    // Avanza la física para un dt en milisegundos. Retorna false si llegó al rango máximo.
    bool step(float dtMs);

    ProjectileDTO toDTO() const;
};

#endif
