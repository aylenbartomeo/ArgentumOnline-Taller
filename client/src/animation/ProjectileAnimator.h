#ifndef PROJECTILE_ANIMATOR_H
#define PROJECTILE_ANIMATOR_H

#include <cstdint>

#include "common/include/dto/Snapshot.h"
// Mantiene la posición visual interpolada de un proyectil entre snapshots
class ProjectileAnimator {
public:
    void update(const ProjectileDTO& dto, uint32_t nowMs);
    float getVirtualX() const { return virtX; }
    float getVirtualY() const { return virtY; }

private:
    float virtX = 0.f, virtY = 0.f;
    float velX = 0.f, velY = 0.f;  // tiles/ms recibida del servidor
    uint32_t lastUpdateMs = 0;
    bool initialized = false;
};

#endif
