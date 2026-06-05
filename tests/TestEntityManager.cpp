#include <gtest/gtest.h>
#include "model/entities/EntityManager.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

static PlayerConfig getTestPlayerConfig() { return {15, 15, 15, 15, 1, 0, 0}; }
static RaceConfig getTestRaceConfig() { return {1.0f, 1.0f, 1.0f}; }
static CharacterClassConfig getTestClassConfig() { return {1.0f, 1.0f, 1.0f, false}; }
static InventoryConfig getTestInventoryConfig() { return {16, 0, 10000, 5000}; }

std::unique_ptr<Player> createTestPlayer(uint32_t entityId, uint32_t dbId, const std::string& name, const ItemRegistry& reg) {
    return std::make_unique<Player>(
        entityId, dbId, name, Race::HUMAN, CharacterClass::WARRIOR, 
        getTestRaceConfig(), getTestClassConfig(), getTestPlayerConfig(), 
        reg, getTestInventoryConfig(), Position{0, 0}
    );
}

TEST(EntityManagerTest, HandlesPlayerLifecycle) {
    EntityManager em;
    ItemRegistry reg("../config/items.toml");

    uint32_t id1 = 100;
    uint32_t id2 = 200;

    uint32_t e1 = em.allocateEntityId();
    uint32_t e2 = em.allocateEntityId();
    auto p1 = createTestPlayer(e1, id1, "PlayerOne", reg);
    auto p2 = createTestPlayer(e2, id2, "PlayerTwo", reg);

    em.registerPlayer(e1, id1, std::move(p1));
    em.registerPlayer(e2, id2, std::move(p2));

    EXPECT_EQ(em.getPlayerCount(), 2);
    EXPECT_FALSE(em.isEmpty());

    // Fallar al duplicar ID de base de datos
    uint32_t e3 = em.allocateEntityId();
    auto p3 = createTestPlayer(e3, id1, "PlayerThree", reg);
    em.registerPlayer(e3, id1, std::move(p3));
    // It shouldn't add a new one if duplicate dbId exists
    EXPECT_EQ(em.getPlayerCount(), 2);

    EXPECT_TRUE(em.removePlayer(id1));
    EXPECT_EQ(em.getPlayerCount(), 1);

    EXPECT_TRUE(em.removePlayer(id2));
    EXPECT_TRUE(em.isEmpty());
}

TEST(EntityManagerTest, RemoveNonExistentPlayerReturnsFalse) {
    EntityManager em;
    EXPECT_FALSE(em.removePlayer(999));
}

TEST(EntityManagerTest, GetPlayerPositionReturnsCurrentPos) {
    EntityManager em;
    ItemRegistry reg("../config/items.toml");
    uint32_t dbId = 100;
    uint32_t e = em.allocateEntityId();
    auto p = createTestPlayer(e, dbId, "PlayerOne", reg);
    p->setPosition(Position{10, 20});
    em.registerPlayer(e, dbId, std::move(p));

    auto posOpt = em.getPlayerPosition(dbId);
    ASSERT_TRUE(posOpt.has_value());
    EXPECT_EQ(posOpt->x, 10);
    EXPECT_EQ(posOpt->y, 20);
}

TEST(EntityManagerTest, GetPlayerUsernameReturnsCorrectName) {
    EntityManager em;
    ItemRegistry reg("../config/items.toml");
    uint32_t dbId = 123;
    uint32_t e = em.allocateEntityId();
    auto p = createTestPlayer(e, dbId, "UsernameTest", reg);
    em.registerPlayer(e, dbId, std::move(p));

    auto nameOpt = em.getPlayerUsername(dbId);
    ASSERT_TRUE(nameOpt.has_value());
    EXPECT_EQ(*nameOpt, "UsernameTest");
}

TEST(EntityManagerTest, GetOnlinePlayerDbIdsReturnsAllActive) {
    EntityManager em;
    ItemRegistry reg("../config/items.toml");
    
    uint32_t e1 = em.allocateEntityId();
    uint32_t e2 = em.allocateEntityId();
    auto p1 = createTestPlayer(e1, 10, "P1", reg);
    auto p2 = createTestPlayer(e2, 20, "P2", reg);
    
    em.registerPlayer(e1, 10, std::move(p1));
    em.registerPlayer(e2, 20, std::move(p2));

    auto ids = em.getOnlinePlayerDbIds();
    EXPECT_EQ(ids.size(), 2u);
    EXPECT_NE(std::find(ids.begin(), ids.end(), 10u), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), 20u), ids.end());
}
