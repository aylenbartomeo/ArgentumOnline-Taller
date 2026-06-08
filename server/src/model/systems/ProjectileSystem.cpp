#include "ProjectileSystem.h"

#include <cmath>

static constexpr float SPEED_TPS = 12.f;
static constexpr float MAX_RANGE = 15.f;
static constexpr uint16_t ARROW_SPRITE = 200;  // EJEMPLO DE ID SPRITE: AJUSTAR!!!!
static constexpr float HIT_RADIUS = 0.6f;      // tiles

ProjectileSystem::ProjectileSystem(Map& map, EntityManager& em, CombatSystem& cs):
        map(map), entityManager(em), combatSystem(cs) {}

uint32_t ProjectileSystem::spawnProjectile(uint32_t ownerDbId, float sx, float sy, float tx,
                                           float ty, uint16_t spriteId, int minDmg, int maxDmg,
                                           bool isMagical, ProjectileType type, float speed,
                                           float maxRange) {
    uint32_t id = nextId++;
    projectiles.emplace_back(id, ownerDbId, sx, sy, tx, ty, speed, maxRange, spriteId, minDmg,
                             maxDmg, isMagical, type);
    return id;
}

void ProjectileSystem::update(float dtMs) {
    for (auto& p: projectiles) {
        if (!p.alive)
            continue;

        bool inRange = p.step(dtMs);
        if (!inRange) {
            p.alive = false;
            continue;
        }

        // Colisión con obstáculos estáticos del mapa
        if (map.playerColision(p.x, p.y)) {
            p.alive = false;
            continue;
        }

        // Colisión con entidades
        uint32_t hitId = 0;
        if (checkCollisionWithEntities(p, hitId)) {
            onProjectileHit(p, hitId);
            p.alive = false;
            continue;
        }
    }

    // Limpiar proyectiles muertos — swap-and-pop para no invalidar índices en mid-loop
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
                                     [](const Projectile& p) { return !p.alive; }),
                      projectiles.end());
}

bool ProjectileSystem::checkCollisionWithEntities(const Projectile& p, uint32_t& hitEntityId) {
    auto checkDist = [&](float ex, float ey, uint32_t id) -> bool {
        float dx = p.x - ex, dy = p.y - ey;
        if (std::sqrt(dx * dx + dy * dy) < HIT_RADIUS) {
            hitEntityId = id;
            return true;
        }
        return false;
    };

    for (const auto& [id, player]: entityManager.getPlayers()) {
        if (player->getDbId() == p.ownerDbId || player->isDead())
            continue;
        auto pos = player->getPosition();
        if (checkDist(pos.x, pos.y, player->getDbId()))
            return true;
    }
    for (const auto& [id, monster]: entityManager.getMonsters()) {
        if (monster->isDead())
            continue;
        auto pos = monster->getPosition();
        if (checkDist(pos.x, pos.y, id))
            return true;
    }
    return false;
}

std::vector<ProjectileDTO> ProjectileSystem::getProjectileDTOs() const {
    std::vector<ProjectileDTO> dtos;
    dtos.reserve(projectiles.size());
    for (const auto& p: projectiles)
        if (p.alive)
            dtos.push_back(p.toDTO());
    return dtos;
}

void ProjectileSystem::onProjectileHit(const Projectile& p, uint32_t hitEntityId) {
    switch (p.type) {
        case ProjectileType::EXPLOSION:
            applyAoeDamage(p);
            break;
        default:
            combatSystem.applyProjectileDamage(p.ownerDbId, hitEntityId, p.minDamage, p.maxDamage,
                                               p.isMagical);
            break;
    }
}

void ProjectileSystem::applyAoeDamage(const Projectile& p) {
    constexpr float AOE_RADIUS = 1.5f;
    auto checkAndDamage = [&](float ex, float ey, uint32_t id) {
        float dx = p.x - ex, dy = p.y - ey;
        if (std::sqrt(dx * dx + dy * dy) <= AOE_RADIUS)
            combatSystem.applyProjectileDamage(p.ownerDbId, id, p.minDamage, p.maxDamage, true);
    };
    for (const auto& [id, player]: entityManager.getPlayers()) {
        if (player->getDbId() == p.ownerDbId || player->isDead())
            continue;
        auto pos = player->getPosition();
        checkAndDamage(pos.x, pos.y, player->getDbId());
    }
    for (const auto& [id, monster]: entityManager.getMonsters()) {
        if (monster->isDead())
            continue;
        auto pos = monster->getPosition();
        checkAndDamage(pos.x, pos.y, id);
    }
}
