#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../common/include/dto/CommandDTO.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

#include "Priest.h"
#include "TestHelpers.h"
#include "World.h"

// =========================================================================
// TEST 1: EL SACERDOTE RESUCITA A UN FANTASMA (/resucitar)
// =========================================================================
TEST(PriestTest, Priest_ResurrectsDeadPlayerSuccessfully) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry, {});

    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    player.handleDeath();
    ASSERT_TRUE(player.isDead());

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::RESPAWN);

    InteractionResult res = sacerdote.handleCommand(player, cmd);

    EXPECT_FALSE(player.isDead());
    EXPECT_GT(player.getHp(), 0u);
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
}

// =========================================================================
// TEST 2: EL SACERDOTE CURA VIDA Y MANÁ AL MÁXIMO
// =========================================================================
TEST(PriestTest, Priest_HealsHpAndManaToMaximum) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry, {});
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    // Le hacemos daño para drenar estadísticas
    player.receiveDamage(5);
    player.consumeMana(10);

    ASSERT_LT(player.getHp(), player.getMaxHp());

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::HEAL);

    InteractionResult res = sacerdote.handleCommand(player, cmd);

    EXPECT_EQ(player.getHp(), player.getMaxHp());
    EXPECT_EQ(player.getMana(), player.getMaxMana());
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
}

// =========================================================================
// TEST 3: COMERCIO - COMPRA EXITOSA DE UN ÍTEM
// =========================================================================
TEST(PriestTest, Priest_AllowsBuyingItemsInStock) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry, {{1001u, 5}});
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    player.addGold(500);

    // Compra directa usando la firma reutilizable del helper
    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::BUY, "Armadura de placas");

    InteractionResult res = sacerdote.handleCommand(player, cmd);

    uint32_t expectedGold = 500u - registry.get_item(1001u)->getPrice();
    EXPECT_EQ(player.getGold(), expectedGold);

    auto slotOpt = player.inspectSlot(0);
    ASSERT_TRUE(slotOpt.has_value());
    EXPECT_EQ(slotOpt->item_id, 1001u);
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
}

// =========================================================================
// TEST 4: UN FANTASMA NO PUEDE COMERCIAR
// =========================================================================
TEST(PriestTest, Priest_DeadPlayerCannotTrade) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry, {{1001u, 5}});
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    player.addGold(500);
    player.handleDeath();

    ASSERT_TRUE(player.isDead());

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::BUY, "Armadura de placas");

    InteractionResult res = sacerdote.handleCommand(player, cmd);

    EXPECT_EQ(player.getGold(), 500u);

    if (player.getSize() > 0) {
        auto slotOpt = player.inspectSlot(0);
        EXPECT_FALSE(slotOpt.has_value());
    }

    EXPECT_EQ(res.status, InteractionStatus::FAILURE);
}

// =========================================================================
// TEST 5: EL SACERDOTE MUESTRA SU CATÁLOGO EXCLUSIVO DE VENTA
// =========================================================================
TEST(PriestTest, Priest_ListStockSuccessfully) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry, {{1001u, 5}, {1002u, 5}});
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::LIST, "");
    InteractionResult res = sacerdote.handleCommand(player, cmd);

    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
    EXPECT_NE(res.msg.find("--- CATÁLOGO DISPONIBLE ---"), std::string::npos);

    EXPECT_NE(res.msg.find("Armadura de placas"), std::string::npos);
    EXPECT_NE(res.msg.find("Tunica azul"), std::string::npos);
}
