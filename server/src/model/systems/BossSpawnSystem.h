#ifndef BOSS_SPAWN_SYSTEM_H
#define BOSS_SPAWN_SYSTEM_H

#include <vector>

#include "../../config/MonsterConfig.h"
#include "../entities/EntityManager.h"
#include "../map/BossZone.h"

#include "RandomNumberGenerator.h"
#include "SpawnSystem.h"  // For SpawnRequest

struct BossSpawnResult {
    size_t zoneIndex;
    SpawnRequest request;
};

class BossSpawnSystem {
private:
    std::vector<BossZone> bossZones;
    const MonsterConfigs* monsterConfigs = nullptr;
    RandomNumberGenerator rng;

    NPCType getRandomBossType() const;

public:
    BossSpawnSystem() = default;

    void setConfigs(const MonsterConfigs* configs) { monsterConfigs = configs; }

    void addBossZone(const BossZoneConfig& config);

    // Tick: verifica cooldowns y genera SpawnRequests
    std::vector<BossSpawnResult> tick(float deltaTime, const EntityManager& em);

    // Notifica muerte de un boss -> inicia cooldown
    void onBossDeath(uint32_t bossEntityId);

    // Registra el entityId del boss recién spawneado
    void registerBossEntity(size_t zoneIndex, uint32_t entityId, NPCType type);

    // Consultas
    bool isBoss(uint32_t entityId, const EntityManager& em) const;
    bool isInAnyBossArea(const Position& pos) const;
    bool isPositionInBossZone(uint32_t bossEntityId, const Position& pos) const;
};

#endif
