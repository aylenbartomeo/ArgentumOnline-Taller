#ifndef SPAWN_ZONE_H
#define SPAWN_ZONE_H

#include <optional>
#include <vector>

#include "../../Map.h"
#include "../../common/utils/position.h"
#include "../../common/utils/types.h"

class SpawnZone {
private:
    int x, y, width, height;
    ZoneType zoneType;
    std::vector<NPCType> allowedMonsterTypes;

    size_t maxMonsters;
    size_t currentMonsterCount;

    float respawnCooldownMs;
    float timeSinceLastSpawnMs;

public:
    SpawnZone(int x, int y, int width, int height, ZoneType type,
              const std::vector<NPCType>& allowedTypes, size_t maxMonsters,
              float cooldownMs = 15000.0f);

    bool contains(const Position& pos) const;

    // Devuelve true si la zona puede spawnear (tiene monstruos permitidos y no alcanzó el límite)
    bool tickCooldown(float deltaMs);

    bool needsSpawn() const;
    void incrementCount();
    void decrementCount();

    size_t getCurrentCount() const { return currentMonsterCount; }
    size_t getMaxMonsters() const { return maxMonsters; }
    void setMaxMonsters(size_t max) { maxMonsters = max; }
    ZoneType getType() const { return zoneType; }

    std::optional<NPCType> getRandomAllowedType() const;

    // Retorna una posición válida dentro de esta zona
    std::optional<Position> getRandomSpawnPosition(int maxAttempts, const Map& map) const;
};

#endif  // SPAWN_ZONE_H
