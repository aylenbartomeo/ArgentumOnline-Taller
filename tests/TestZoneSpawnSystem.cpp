#include <gtest/gtest.h>

#include "model/entities/EntityManager.h"
#include "model/systems/ZoneSpawnSystem.h"
#include "Map.h"

TEST(ZoneSpawnSystemTest, SpawnsMonsterWhenCooldownElapsed) {
    MonsterConfigs configs;
    MonsterConfig cfg;
    cfg.maxHealth = 10;
    cfg.zone = "normal";
    configs[NPCType::GOBLIN] = cfg;

    ZoneSpawnSystem spawnSystem(configs, 15000.0f, 100);
    Map map;
    map.setDimensions(20, 15);
    spawnSystem.initializeZones(map);

    // 1. El primer tick dispara un spawn porque el cooldown inicial está lleno
    auto reqs1 = spawnSystem.tick(2000.0f, map);
    EXPECT_FALSE(reqs1.empty());

    // 2. Update < cooldown, no debería spawnear
    auto reqs2 = spawnSystem.tick(14000.0f, map);
    EXPECT_TRUE(reqs2.empty());

    // 3. Update restante para alcanzar el cooldown (14000 + 1000 = 15000)
    auto reqs3 = spawnSystem.tick(1000.0f, map);
    EXPECT_EQ(reqs3.size(), 1u);
}

TEST(ZoneSpawnSystemTest, StopsWhenWorldRejectsSpawn) {
    MonsterConfigs configs;
    MonsterConfig cfg;
    cfg.maxHealth = 10;
    cfg.zone = "normal";
    configs[NPCType::GOBLIN] = cfg;

    ZoneSpawnSystem spawnSystem(configs, 15000.0f, 100);
    Map map;
    map.setDimensions(20, 15);
    spawnSystem.initializeZones(map);

    for (int x = 0; x < 20; ++x) {
        for (int y = 0; y < 15; ++y) {
            map.setObstacleInGrid(x, y, true);
        }
    }

    auto reqs = spawnSystem.tick(16000.0f, map);
    EXPECT_TRUE(reqs.empty());
}
