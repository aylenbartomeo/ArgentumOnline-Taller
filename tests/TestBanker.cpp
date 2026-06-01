#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

#include "Banker.h"
#include "GlobalBank.h"

// Usamos el constructor de TEST de tu clase Player (recibe FormulaEngine)
static Player makeBankerTestPlayer(uint32_t id = 1) {
    std::string name = "TestPlayer";
    RaceConfig race = {1.0f, 1.0f, 1.0f};
    CharacterClassConfig cls = {1.0f, 1.0f, 1.0f, false};
    PlayerConfig cfg = {15, 15, 15, 15, 1, 0, 0};
    return Player(id, id, name, Race::HUMAN, CharacterClass::WARRIOR, race, cls, cfg,
                  FormulaEngine::getInstance());
}

// =========================================================================
// TEST 1: ENTRADA Y SALIDA DE ORO EN EL BANQUERO (/depositar oro X)
// =========================================================================
TEST(BankerTest, Banker_HandlesGoldDepositAndWithdrawCommands) {
    ItemRegistry registry("../config/items.toml");
    GlobalBank bancoInstance;
    Banker banquero(1, {0, 0}, bancoInstance, registry);
    Player player = makeBankerTestPlayer();

    player.addGold(1000u);

    // 1. Ejecutamos el comando unificado: /depositar oro 400
    NpcCommandDTO depCmd = {NpcCommandType::DEPOSIT, "oro 400"};
    InteractionResult depRes = banquero.handleCommand(player, depCmd);

    // Validaciones del depósito
    EXPECT_EQ(depRes.status, InteractionStatus::SUCCESS);
    EXPECT_EQ(player.getGold(), 600u);
    EXPECT_EQ(bancoInstance.getBankGold(player.getDbId()), 400u);

    // 2. Ejecutamos el comando unificado: /retirar oro 200
    NpcCommandDTO withCmd = {NpcCommandType::WITHDRAW, "oro 200"};
    InteractionResult withRes = banquero.handleCommand(player, withCmd);

    // Validaciones de retiro
    EXPECT_EQ(withRes.status, InteractionStatus::SUCCESS);
    // El jugador debería recuperar sus 200 monedas (600 + 200 = 800)
    EXPECT_EQ(player.getGold(), 800u);
    EXPECT_EQ(bancoInstance.getBankGold(player.getDbId()), 200u);
}

// =========================================================================
// TEST 2: DEPOSITAR UN ÍTEM DE LA MOCHILA AL BANCO (/depositar <id>)
// =========================================================================
TEST(BankerTest, Banker_DepositsItemFromInventorySuccessfully) {
    ItemRegistry registry("../config/items.toml");
    GlobalBank bancoInstance;
    Banker banquero(1, {0, 0}, bancoInstance, registry);
    Player player = makeBankerTestPlayer();

    // Seteamos la precondición: le damos la Espada (ID: 4001) al jugador
    player.addItem(4001u, 1);
    ASSERT_TRUE(player.inspectSlot(0).has_value());

    // Mandamos el comando unificado enviando el ID del ítem como argumento string
    NpcCommandDTO cmd = {NpcCommandType::DEPOSIT, "4001"};
    InteractionResult res = banquero.handleCommand(player, cmd);

    // VALIDACIONES:
    // 0. Verificamos que el depósito fue exitoso
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
    // 1. El ítem ya no está en la mochila del jugador (slot liberado)
    auto slotOpt = player.inspectSlot(0);
    EXPECT_FALSE(slotOpt.has_value());

    // 2. Verificamos que el ítem fue extraído por el banquero y depositado en la bóveda global
    // Intentamos retirarlo directamente del banco para probar que quedó ahí guardado
    uint16_t cantidadEnBanco = bancoInstance.withdrawItemById(player.getDbId(), 4001u, 1);
    EXPECT_EQ(cantidadEnBanco, 1);
}

// =========================================================================
// TEST 3: RETIRAR UN ÍTEM DE LA BÓVEDA POR ID (/retirar <id>)
// =========================================================================
TEST(BankerTest, Banker_WithdrawsItemByIdSuccessfully) {
    ItemRegistry registry("../config/items.toml");
    GlobalBank bancoInstance;
    Banker banquero(1, {0, 0}, bancoInstance, registry);
    Player player = makeBankerTestPlayer();

    // Forzamos la precondición directamente en la bóveda del banco usando la API limpia
    bancoInstance.depositItem(player.getDbId(), 4001u, 1);

    // El jugador intenta retirar el ítem ejecutando el comando con el ID
    NpcCommandDTO cmd = {NpcCommandType::WITHDRAW, "4001"};
    InteractionResult res = banquero.handleCommand(player, cmd);

    // VALIDACIONES:
    // 0. Verificamos que el retiro fue exitoso
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
    // 1. El ítem ingresó de forma segura al inventario del player en el slot 0
    auto slotOpt = player.inspectSlot(0);
    ASSERT_TRUE(slotOpt.has_value());
    EXPECT_EQ(slotOpt->item_id, 4001u);
    EXPECT_EQ(slotOpt->amount, 1);
}

// =========================================================================
// TEST 4: ROLLBACK ATÓMICO SI LA MOCHILA ESTÁ LLENA AL RETIRAR
// =========================================================================
TEST(BankerTest, Banker_WithdrawWithFullInventoryDoesNotLoseItem) {
    ItemRegistry registry("../config/items.toml");
    GlobalBank bancoInstance;
    Banker banquero(1, {0, 0}, bancoInstance, registry);
    Player player = makeBankerTestPlayer();

    // Precondición 1: Guardamos una Espada (4001) en el banco
    bancoInstance.depositItem(player.getDbId(), 4001u, 1);

    // Precondición 2: Llenamos por completo la mochila del jugador con IDs únicos
    uint8_t sizeMochila = player.getSize();
    for (uint8_t i = 0; i < sizeMochila; ++i) {
        player.addItem(1001u + i, 1);
    }

    // Intentamos retirar la espada del banco
    NpcCommandDTO cmd = {NpcCommandType::WITHDRAW, "4001"};
    InteractionResult res = banquero.handleCommand(player, cmd);

    // VALIDACIONES CRÍTICAS:
    // 0. Verificamos que el retiro fue rechazado (mochila llena)
    EXPECT_EQ(res.status, InteractionStatus::FAILURE);
    // 1. ¡Rollback exitoso! El ítem NO se destruyó ni se perdió; se mantuvo a salvo dentro del
    // banco
    uint16_t cantidadEnBanco = bancoInstance.withdrawItemById(player.getDbId(), 4001u, 1);
    EXPECT_EQ(cantidadEnBanco, 1);
}
