#include "ProjectileAnimator.h"

void ProjectileAnimator::update(const ProjectileDTO& dto, uint32_t nowMs) {
    // Snap a la posición autoritativa del servidor
    virtX = dto.x;
    virtY = dto.y;
    // Guardamos velocidad en tiles/ms para extrapolación
    velX = dto.velX / 1000.f;
    velY = dto.velY / 1000.f;
    lastUpdateMs = nowMs;
    spriteId = dto.spriteId;
    initialized = true;
}

void ProjectileAnimator::extrapolate(uint32_t nowMs) {
    if (!initialized)
        return;
    const float dt = static_cast<float>(nowMs - lastUpdateMs);
    virtX += velX * dt;
    virtY += velY * dt;
    lastUpdateMs = nowMs;
}
