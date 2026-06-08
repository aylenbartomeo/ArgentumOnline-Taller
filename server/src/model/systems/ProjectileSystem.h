#ifndef PROJECTILE_SYSTEM_H
#define PROJECTILE_SYSTEM_H

#include <vector>

#include "model/entities/EntityManager.h"
#include "model/entities/Projectile.h"
#include "model/systems/CombatSystem.h"

#include "Map.h"

class ProjectileSystem {
public:
    ProjectileSystem(Map& map, EntityManager& em, CombatSystem& cs);
    uint32_t spawnProjectile(uint32_t ownerDbId, float startX, float startY, float targetX,
                             float targetY);
    void update(float dtMs);
    std::vector<ProjectileDTO> getProjectileDTOs() const;

private:
    Map& map;
    EntityManager& entityManager;
    CombatSystem& combatSystem;
    std::vector<Projectile> projectiles;
    uint32_t nextId = 1;
    bool checkCollisionWithEntities(const Projectile& p, uint32_t& hitEntityId);
};

#endif
