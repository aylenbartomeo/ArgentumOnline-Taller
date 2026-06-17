#include "BossZone.h"

BossZone::BossZone(const BossZoneConfig& cfg) : config(cfg) {
    bossAlive = false;
    cooldownRemainingMs = 0; // Spawnea inmediatamente al iniciar el server
    bossEntityId = 0;
}

bool BossZone::isInsideArea(const Position& pos) const {
    int px = pos.x;
    int py = pos.y;
    int ax = config.x;
    int ay = config.y;
    int aw = config.width;
    int ah = config.height;
    
    return (px >= ax && px <= ax + aw && py >= ay && py <= ay + ah);
}

bool BossZone::isBossAlive() const {
    return bossAlive;
}

void BossZone::markBossDead() {
    bossAlive = false;
    bossEntityId = 0;
    cooldownRemainingMs = config.respawnCooldownMs;
}

void BossZone::setBossEntityId(uint32_t id, NPCType type) {
    bossEntityId = id;
    currentBossType = type;
    bossAlive = true;
    cooldownRemainingMs = 0;
}

uint32_t BossZone::getBossEntityId() const {
    return bossEntityId;
}

NPCType BossZone::getCurrentBossType() const {
    return currentBossType;
}

Position BossZone::getSpawnPosition() const {
    return Position{config.spawnX, config.spawnY};
}

float BossZone::getRespawnCooldownMs() const {
    return config.respawnCooldownMs;
}

bool BossZone::tickCooldown(float deltaMs) {
    if (bossAlive) {
        return false;
    }
    
    cooldownRemainingMs -= deltaMs;
    if (cooldownRemainingMs <= 0) {
        cooldownRemainingMs = 0;
        return true; // Cooldown finalizado, debe respawnear
    }
    
    return false;
}
