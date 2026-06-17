#ifndef BOSSZONE_H
#define BOSSZONE_H

#include <cstdint>

#include "position.h"
#include "types.h"

struct BossZoneConfig {
    int x;
    int y;
    int width;
    int height;
    int spawnX;
    int spawnY;
    float respawnCooldownMs;
};

class BossZone {
private:
    BossZoneConfig config;
    uint32_t bossEntityId = 0;  // 0 = no hay boss vivo
    bool bossAlive = false;
    float cooldownRemainingMs = 0;
    NPCType currentBossType =
            NPCType::BOSS_BALROG;  // Tipo actual (para cuando está vivo o por spawnear)

public:
    explicit BossZone(const BossZoneConfig& cfg);

    bool isInsideArea(const Position& pos) const;
    bool isBossAlive() const;
    void markBossDead();  // Inicia el cooldown
    void setBossEntityId(uint32_t id, NPCType type);
    uint32_t getBossEntityId() const;
    NPCType getCurrentBossType() const;
    Position getSpawnPosition() const;
    float getRespawnCooldownMs() const;

    // Retorna true si el cooldown terminó y hay que respawnear
    bool tickCooldown(float deltaMs);
};

#endif
