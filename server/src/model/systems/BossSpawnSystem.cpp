#include "BossSpawnSystem.h"

#include <algorithm>

#include "../entities/Monster.h"

NPCType BossSpawnSystem::getRandomBossType() const {
    int r = rng(0, 3);  // 0 to 3 inclusive
    switch (r) {
        case 0:
            return NPCType::BOSS_BALROG;
        case 1:
            return NPCType::BOSS_TITAN;
        case 2:
            return NPCType::BOSS_COLOSO;
        case 3:
            return NPCType::BOSS_ARACNE;
        default:
            return NPCType::BOSS_BALROG;
    }
}

void BossSpawnSystem::addBossZone(const BossZoneConfig& config) { bossZones.emplace_back(config); }

std::vector<BossSpawnResult> BossSpawnSystem::tick(float deltaTime, const EntityManager& em) {
    std::vector<BossSpawnResult> results;
    if (!monsterConfigs)
        return results;

    for (size_t i = 0; i < bossZones.size(); ++i) {
        auto& zone = bossZones[i];

        // Verificar si el boss actual sigue vivo (por si murio sin avisar, ej desconexion/limpieza)
        if (zone.isBossAlive()) {
            if (em.getMonsters().find(zone.getBossEntityId()) == em.getMonsters().end()) {
                zone.markBossDead();
            }
        }

        // Tick de cooldown
        if (zone.tickCooldown(deltaTime)) {
            NPCType newType = getRandomBossType();

            SpawnRequest req;
            req.type = newType;
            req.pos = zone.getSpawnPosition();

            auto it = monsterConfigs->find(newType);
            if (it != monsterConfigs->end()) {
                req.config = &it->second;

                BossSpawnResult res;
                res.zoneIndex = i;
                res.request = req;
                results.push_back(res);
            }
        }
    }
    return results;
}

void BossSpawnSystem::registerBossEntity(size_t zoneIndex, uint32_t entityId, NPCType type) {
    if (zoneIndex < bossZones.size()) {
        bossZones[zoneIndex].setBossEntityId(entityId, type);
    }
}

void BossSpawnSystem::onBossDeath(uint32_t bossEntityId) {
    auto it = std::find_if(bossZones.begin(), bossZones.end(), [&](const BossZone& zone) {
        return zone.getBossEntityId() == bossEntityId;
    });
    if (it != bossZones.end()) {
        it->markBossDead();
    }
}

bool BossSpawnSystem::isBoss(uint32_t entityId, const EntityManager& em) const {
    auto it = em.getMonsters().find(entityId);
    if (it == em.getMonsters().end())
        return false;
    const Monster* monster = it->second.get();
    return isBossType(monster->getType());
}

bool BossSpawnSystem::isInAnyBossArea(const Position& pos) const {
    return std::any_of(bossZones.begin(), bossZones.end(),
                       [&](const BossZone& zone) { return zone.isInsideArea(pos); });
}

bool BossSpawnSystem::isPositionInBossZone(uint32_t bossEntityId, const Position& pos) const {
    auto it = std::find_if(bossZones.begin(), bossZones.end(), [&](const BossZone& zone) {
        return zone.getBossEntityId() == bossEntityId;
    });
    if (it != bossZones.end()) {
        return it->isInsideArea(pos);
    }
    return false;
}
