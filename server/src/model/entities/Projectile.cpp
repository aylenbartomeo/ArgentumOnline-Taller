#include "Projectile.h"

#include <cmath>

Projectile::Projectile(uint32_t id, uint32_t owner, float sx, float sy, float tx, float ty,
                       float speedTPS, float maxRng, uint16_t sprite, int minDmg, int maxDmg,
                       bool magical, ProjectileType pType, IHitEffect* hitEffect):
        id(id),
        ownerDbId(owner),
        x(sx),
        y(sy),
        maxRange(maxRng),
        distTraveled(0.f),
        spriteId(sprite),
        minDamage(minDmg),
        maxDamage(maxDmg),
        isMagical(magical),
        type(pType),
        capturedHitEffect(hitEffect) {
    float dx = tx - sx, dy = ty - sy;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.001f)
        len = 1.f;
    float speedTPMs = speedTPS / 1000.f;  // tiles/ms
    velX = (dx / len) * speedTPMs;
    velY = (dy / len) * speedTPMs;
}

bool Projectile::step(float dtMs) {
    float mx = velX * dtMs;
    float my = velY * dtMs;
    x += mx;
    y += my;
    distTraveled += std::sqrt(mx * mx + my * my);
    return distTraveled < maxRange;
}

ProjectileDTO Projectile::toDTO() const {
    // velX/Y se mandan en tiles/seg para que el cliente calcule extrapolación
    return {id, x, y, velX * 1000.f, velY * 1000.f, spriteId};
}
