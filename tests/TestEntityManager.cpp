#include <algorithm>

#include <gtest/gtest.h>

#include "model/entities/EntityManager.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

#include "TestHelpers.h"

// =========================================================================
// PRUEBAS DE CICLO DE VIDA Y GESTIÓN DE ENTIDADES
// =========================================================================

TEST(EntityManagerTest, HandlesPlayerLifecycle) {
    EntityManager em;
    ItemRegistry reg("../config/items.toml");

    uint32_t id1 = 100;
    uint32_t id2 = 200;

    uint32_t e1 = em.allocateEntityId();
    uint32_t e2 = em.allocateEntityId();

    // 🚀 Instanciación directa y limpia delegada a los builders de TestUtils
    auto p1 = std::make_unique<Player>(TestUtils::makeTestPlayer(id1, "PlayerOne"));
    auto p2 = std::make_unique<Player>(TestUtils::makeTestPlayer(id2, "PlayerTwo"));

    em.registerPlayer(e1, id1, std::move(p1));
    em.registerPlayer(e2, id2, std::move(p2));

    EXPECT_EQ(em.getPlayerCount(), 2);
    EXPECT_FALSE(em.isEmpty());

    // Fallar al duplicar ID de base de datos
    uint32_t e3 = em.allocateEntityId();
    auto p3 = std::make_unique<Player>(TestUtils::makeTestPlayer(id1, "PlayerThree"));
    em.registerPlayer(e3, id1, std::move(p3));

    // No debería agregar un nuevo jugador si ya existe el dbId duplicado
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
    uint32_t dbId = 100;
    uint32_t e = em.allocateEntityId();

    auto p = std::make_unique<Player>(TestUtils::makeTestPlayer(dbId, "PlayerOne"));
    p->setPosition(Position{10, 20});
    em.registerPlayer(e, dbId, std::move(p));

    auto posOpt = em.getPlayerPosition(dbId);
    ASSERT_TRUE(posOpt.has_value());
    EXPECT_EQ(posOpt->x, 10);
    EXPECT_EQ(posOpt->y, 20);
}

TEST(EntityManagerTest, GetPlayerUsernameReturnsCorrectName) {
    EntityManager em;
    uint32_t dbId = 123;
    uint32_t e = em.allocateEntityId();

    auto p = std::make_unique<Player>(TestUtils::makeTestPlayer(dbId, "UsernameTest"));
    em.registerPlayer(e, dbId, std::move(p));

    auto nameOpt = em.getPlayerUsername(dbId);
    ASSERT_TRUE(nameOpt.has_value());
    EXPECT_EQ(*nameOpt, "UsernameTest");
}

TEST(EntityManagerTest, GetOnlinePlayerDbIdsReturnsAllActive) {
    EntityManager em;

    uint32_t e1 = em.allocateEntityId();
    uint32_t e2 = em.allocateEntityId();

    auto p1 = std::make_unique<Player>(TestUtils::makeTestPlayer(10, "P1"));
    auto p2 = std::make_unique<Player>(TestUtils::makeTestPlayer(20, "P2"));

    em.registerPlayer(e1, 10, std::move(p1));
    em.registerPlayer(e2, 20, std::move(p2));

    auto ids = em.getOnlinePlayerDbIds();
    EXPECT_EQ(ids.size(), 2u);
    EXPECT_NE(std::find(ids.begin(), ids.end(), 10u), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), 20u), ids.end());
}
