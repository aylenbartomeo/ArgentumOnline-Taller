#include <gtest/gtest.h>
#include "model/systems/SpawnSystem.h"
#include "model/entities/EntityManager.h"
#include "Map.h"

TEST(SpawnSystemTest, SpawnsMonsterWhenCooldownElapsed) {
    MonsterConfigs configs;
    MonsterConfig cfg;
    cfg.maxHealth = 10;
    configs[NPCType::GOBLIN] = cfg;

    SpawnSystem spawnSystem(configs, 5000.0f, 100);
    Map map;
    map.setDimensions(20, 15);
    EntityManager entityManager;

    // 1. Update < cooldown
    auto reqs1 = spawnSystem.tick(2000.0f, 0, map, entityManager);
    EXPECT_TRUE(reqs1.empty());

    // 2. Update >= cooldown
    auto reqs2 = spawnSystem.tick(3500.0f, 0, map, entityManager);
    EXPECT_EQ(reqs2.size(), 1u);
}

TEST(SpawnSystemTest, StopsWhenWorldRejectsSpawn) {
    MonsterConfigs configs;
    MonsterConfig cfg;
    cfg.maxHealth = 10;
    configs[NPCType::GOBLIN] = cfg;

    SpawnSystem spawnSystem(configs, 5000.0f, 100);
    Map map;
    map.setDimensions(20, 15);
    for (int x = 0; x < 20; ++x) {
        for (int y = 0; y < 15; ++y) {
            map.setObstacleInGrid(x, y, true);
        }
    }
    EntityManager entityManager;

    auto reqs = spawnSystem.tick(6000.0f, 0, map, entityManager);
    EXPECT_TRUE(reqs.empty());
}
