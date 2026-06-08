#ifndef SPAWN_SYSTEM_H
#define SPAWN_SYSTEM_H

#include <optional>
#include <vector>

#include "../../Map.h"
#include "../../config/MonsterConfig.h"
#include "../entities/EntityManager.h"

struct SpawnRequest {
    NPCType type;
    Position pos;
    const MonsterConfig* config;
};

class SpawnSystem {
private:
    MonsterConfigs monsterConfigs;
    float respawnCooldownMs;
    float timeSinceLastSpawnMs;
    size_t maxMonsters;

    std::optional<Position> findValidSpawnPosition(int maxAttempts, const Map& map,
                                                   const EntityManager& entityManager) const;

public:
    explicit SpawnSystem(MonsterConfigs configs = {}, float cooldownMs = 5000.0f,
                         size_t maxMon = 100);

    std::vector<SpawnRequest> tick(float deltaTime, size_t currentMonsterCount, const Map& map,
                                   const EntityManager& entityManager);

    std::vector<SpawnRequest> getInitialSpawns(const Map& map) const;

    const MonsterConfigs& getConfigs() const { return monsterConfigs; }
};

#endif  // SPAWN_SYSTEM_H
