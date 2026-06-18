#include "ProjectileSystem.h"

#include <cmath>

#include "../interfaces/CombatStrategies.h"
#include "../items/Weapon.h"

static constexpr float SPEED_TPS = 12.f;
static constexpr float MAX_RANGE = 15.f;
static constexpr uint16_t ARROW_SPRITE = 200;  // EJEMPLO DE ID SPRITE: AJUSTAR!!!!
static constexpr float HIT_RADIUS = 0.6f;      // tiles

ProjectileSystem::ProjectileSystem(Map& map, EntityManager& em, CombatSystem& cs):
        map(map), entityManager(em), combatSystem(cs) {}

uint32_t ProjectileSystem::spawnProjectile(uint32_t ownerDbId, float sx, float sy, float tx,
                                           float ty, uint16_t spriteId, int minDmg, int maxDmg,
                                           bool isMagical, IHitEffect* hitEffect,
                                           ProjectileType type, float speed, float maxRange) {
    uint32_t id = nextId++;
    // Captura el hitEffect en el momento del disparo (snapshot)
    projectiles.emplace_back(id, ownerDbId, sx, sy, tx, ty, speed, maxRange, spriteId, minDmg,
                             maxDmg, isMagical, type, hitEffect);
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

        if (map.isTileSolid(p.x, p.y)) {
            p.alive = false;
            continue;
        }

        // Colisión con entidades
        uint32_t hitId = 0;
        if (checkCollisionWithEntities(p, hitId)) {
            onProjectileHit(p, hitId);
            p.alive = false;
        }
    }

    // Limpiar proyectiles muertos
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
        if (checkDist(pos.x, pos.y, id))
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

static AttackParams buildAttackParams(const Projectile& p, const CombatModifiers& modifiers) {
    return AttackParams{static_cast<uint16_t>(p.minDamage),
                        static_cast<uint16_t>(p.maxDamage),
                        999,
                        0,
                        p.isMagical,
                        modifiers.attackBonus,
                        modifiers.defenseBonus};
}

void ProjectileSystem::onProjectileHit(const Projectile& p, uint32_t hitEntityId) {
    switch (p.type) {
        case ProjectileType::EXPLOSION:
            applyAoeDamage(p);
            break;
        default:
            applySingleTargetDamage(p, hitEntityId);
            break;
    }
}

void ProjectileSystem::applySingleTargetDamage(const Projectile& p, uint32_t hitEntityId) {
    // Resolver entidades
    Player* attacker = entityManager.getPlayer(p.ownerDbId);
    if (!attacker)
        return;

    Attackable* target = entityManager.findAttackable(hitEntityId);
    if (!target || target->isDead())
        return;

    // Guardia de zona segura del target (el proyectil ya cruzó el mapa)
    if (map.isSafeZone(target->getPosition().x, target->getPosition().y))
        return;

    Player* targetPlayer = dynamic_cast<Player*>(target);
    uint32_t targetClanId = targetPlayer ? targetPlayer->getDbId() : hitEntityId;

    if (combatSystem.areClanmates(p.ownerDbId, targetClanId))
        return;

    // if (combatSystem.isFairPlayEnforced() &&
    //     (!attacker->canEngageInCombatWith(*target) || !target->canEngageInCombatWith(*attacker)))
    //     { return;
    // }

    // Calcular modificadores de clan en el momento del impacto
    CombatModifiers modifiers = combatSystem.buildModifiers(p.ownerDbId, target);

    // Usar el hitEffect capturado al momento del disparo.
    // Si el jugador cambió de arma en vuelo, el efecto sigue siendo el del arma original.
    // Si no hay efecto capturado (legacy / spawn sin arma), degradar a daño por parámetros.
    if (!p.capturedHitEffect) {
        AttackParams params = buildAttackParams(p, modifiers);
        combatSystem.applyDamageEffect(*attacker, *target, params);
        return;
    }

    // Necesitamos el Weapon para pasarlo a onProjectileHit; si el jugador cambió de arma
    // en vuelo, usamos el arma actual como contexto de stats (rango ya no importa aquí).
    const Weapon* weapon = attacker->getEquippedWeapon();
    if (!weapon) {
        AttackParams params = buildAttackParams(p, modifiers);
        combatSystem.applyDamageEffect(*attacker, *target, params);
        return;
    }

    combatSystem.onProjectileHit(*attacker, *target, p.capturedHitEffect, modifiers, *weapon);
}

void ProjectileSystem::applyAoeDamage(const Projectile& p) {
    constexpr float AOE_RADIUS = 1.5f;

    Player* attacker = entityManager.getPlayer(p.ownerDbId);
    if (!attacker)
        return;

    // Contexto del arma actual (para pasar stats de rango a onProjectileHit; el rango no
    // se valida en el impacto de proyectil, así que usar el arma actual es aceptable).
    const Weapon* weapon = attacker->getEquippedWeapon();

    auto checkAndDamage = [&](float ex, float ey, uint32_t id) {
        float dx = p.x - ex, dy = p.y - ey;
        if (std::sqrt(dx * dx + dy * dy) > AOE_RADIUS)
            return;

        Attackable* target = entityManager.findAttackable(id);
        if (!target || target->isDead())
            return;

        if (map.isSafeZone(target->getPosition().x, target->getPosition().y))
            return;

        Player* targetPlayer = dynamic_cast<Player*>(target);
        uint32_t targetClanId = targetPlayer ? targetPlayer->getDbId() : id;

        if (combatSystem.areClanmates(p.ownerDbId, targetClanId))
            return;

        // if (combatSystem.isFairPlayEnforced() && (!attacker->canEngageInCombatWith(*target) ||
        //                                           !target->canEngageInCombatWith(*attacker))) {
        //     return;
        // }
        CombatModifiers modifiers = combatSystem.buildModifiers(p.ownerDbId, target);

        if (p.capturedHitEffect && weapon) {
            combatSystem.onProjectileHit(*attacker, *target, p.capturedHitEffect, modifiers,
                                         *weapon);
        } else {
            AttackParams params = buildAttackParams(p, modifiers);
            combatSystem.applyDamageEffect(*attacker, *target, params);
        }
    };

    for (const auto& [id, player]: entityManager.getPlayers()) {
        if (player->getDbId() == p.ownerDbId || player->isDead())
            continue;
        auto pos = player->getPosition();
        checkAndDamage(pos.x, pos.y, id);
    }
    for (const auto& [id, monster]: entityManager.getMonsters()) {
        if (monster->isDead())
            continue;
        auto pos = monster->getPosition();
        checkAndDamage(pos.x, pos.y, id);
    }
}
