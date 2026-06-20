#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

#include "Banker.h"
#include "GlobalBank.h"
#include "TestHelpers.h"

// =========================================================================
// TEST 1: ENTRADA Y SALIDA DE ORO EN EL BANQUERO
// =========================================================================
TEST(BankerTest, Banker_HandlesGoldDepositAndWithdrawCommands) {
    ItemRegistry registry("../config/items.toml");
    GlobalBank bancoInstance;
    Banker banquero(1, {0, 0}, bancoInstance, registry);

    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    player.addGold(1000u);

    NpcCommandDTO depCmd = TestUtils::createTestCommand(NpcCommandType::DEPOSIT, "oro 400");
    InteractionResult depRes = banquero.handleCommand(player, depCmd);

    EXPECT_EQ(depRes.status, InteractionStatus::SUCCESS);
    EXPECT_EQ(player.getGold(), 600u);
    EXPECT_EQ(bancoInstance.getBankGold(player.getDbId()), 400u);

    NpcCommandDTO withCmd = TestUtils::createTestCommand(NpcCommandType::WITHDRAW, "oro 200");
    InteractionResult withRes = banquero.handleCommand(player, withCmd);

    EXPECT_EQ(withRes.status, InteractionStatus::SUCCESS);
    EXPECT_EQ(player.getGold(), 800u);
    EXPECT_EQ(bancoInstance.getBankGold(player.getDbId()), 200u);
}

// =========================================================================
// TEST 2: DEPOSITAR UN ÍTEM DE LA MOCHILA AL BANCO
// =========================================================================
TEST(BankerTest, Banker_DepositsItemFromInventorySuccessfully) {
    ItemRegistry registry("../config/items.toml");
    GlobalBank bancoInstance;
    Banker banquero(1, {0, 0}, bancoInstance, registry);
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    player.addItem(2000u, 1);
    ASSERT_TRUE(player.inspectSlot(0).has_value());

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::DEPOSIT, "Espada");
    InteractionResult res = banquero.handleCommand(player, cmd);

    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
    EXPECT_FALSE(player.inspectSlot(0).has_value());

    uint16_t cantidadEnBanco = bancoInstance.withdrawItemById(player.getDbId(), 2000u, 1);
    EXPECT_EQ(cantidadEnBanco, 1);
}

// =========================================================================
// TEST 3: RETIRAR UN ÍTEM DE LA BÓVEDA POR ID
// =========================================================================
TEST(BankerTest, Banker_WithdrawsItemByIdSuccessfully) {
    ItemRegistry registry("../config/items.toml");
    GlobalBank bancoInstance;
    Banker banquero(1, {0, 0}, bancoInstance, registry);
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    bancoInstance.depositItem(player.getDbId(), 2000u, 1);

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::WITHDRAW, "Espada");
    InteractionResult res = banquero.handleCommand(player, cmd);

    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
    auto slotOpt = player.inspectSlot(0);
    ASSERT_TRUE(slotOpt.has_value());
    EXPECT_EQ(slotOpt->item_id, 2000u);
    EXPECT_EQ(slotOpt->amount, 1);
}

// =========================================================================
// TEST 4: ROLLBACK ATÓMICO SI LA MOCHILA ESTÁ LLENA AL RETIRAR
// =========================================================================
TEST(BankerTest, Banker_WithdrawWithFullInventoryDoesNotLoseItem) {
    ItemRegistry registry("../config/items.toml");
    GlobalBank bancoInstance;
    Banker banquero(1, {0, 0}, bancoInstance, registry);
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    bancoInstance.depositItem(player.getDbId(), 2000u, 1);

    uint8_t sizeMochila = player.getSize();
    for (uint8_t i = 0; i < sizeMochila; ++i) {
        player.addItem(1001u + i, 1);
    }

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::WITHDRAW, "Espada");
    InteractionResult res = banquero.handleCommand(player, cmd);

    EXPECT_EQ(res.status, InteractionStatus::FAILURE);
    uint16_t cantidadEnBanco = bancoInstance.withdrawItemById(player.getDbId(), 2000u, 1);
    EXPECT_EQ(cantidadEnBanco, 1);
}

// =========================================================================
// TEST 5: EL BANQUERO LISTA EXTRACTO INDIVIDUAL
// =========================================================================
TEST(BankerTest, Banker_ListAccountBalancesAndItemsSuccessfully) {
    ItemRegistry registry("../config/items.toml");
    GlobalBank bancoInstance;
    Banker banquero(1, {0, 0}, bancoInstance, registry);
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    uint32_t playerId = player.getDbId();
    bancoInstance.depositGold(playerId, 1500u);
    bancoInstance.depositItem(playerId, 2000u, 1);
    bancoInstance.depositItem(playerId, 2000u, 1);
    bancoInstance.depositItem(playerId, 2000u, 1);

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::LIST);
    InteractionResult res = banquero.handleCommand(player, cmd);

    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
    EXPECT_NE(res.msg.find("--- EXTRACTO BANCARIO ---"), std::string::npos);
    EXPECT_NE(res.msg.find("1500 monedas"), std::string::npos);
    EXPECT_NE(res.msg.find("Espada"), std::string::npos);
    EXPECT_NE(res.msg.find("[Cant: 3]"), std::string::npos);
}
