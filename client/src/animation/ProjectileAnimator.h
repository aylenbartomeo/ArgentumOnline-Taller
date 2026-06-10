#ifndef PROJECTILE_ANIMATOR_H
#define PROJECTILE_ANIMATOR_H

#include <cstdint>

#include "common/include/dto/Snapshot.h"
// Mantiene la posición visual interpolada de un proyectil entre snapshots
class ProjectileAnimator {
public:
    void update(const ProjectileDTO& dto, uint32_t nowMs);

    // Avanza la posición virtual según la velocidad recibida
    void extrapolate(uint32_t nowMs);


    float getVirtualX() const { return virtX; }
    float getVirtualY() const { return virtY; }
    uint16_t getSpriteId() const { return spriteId; }
    float getVelX() const { return velX; }
    float getVelY() const { return velY; }

    int lastPixelX = 0;
    int lastPixelY = 0;

private:
    float virtX = 0.f, virtY = 0.f;
    float velX = 0.f, velY = 0.f;  // tiles/ms recibida del servidor
    uint32_t lastUpdateMs = 0;
    uint16_t spriteId = 0;
    bool initialized = false;
};

#endif
