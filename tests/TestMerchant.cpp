#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

#include "Merchant.h"
#include "World.h"

// Helper para armar comandos de red simulados
NpcCommandDTO createTestCommand(NpcCommandType type, const std::string& itemIdStr) {
    NpcCommandDTO dto;
    dto.type = type;
    dto.arg = itemIdStr;
    return dto;
}

// Helper: crea un Player base para tests
static Player makeTestPlayer(uint32_t id = 1) {
    std::string name = "TestPlayer";
    RaceConfig race = {1.0f, 1.0f, 1.0f};
    CharacterClassConfig cls = {1.0f, 1.0f, 1.0f, false};
    PlayerConfig cfg = {15, 15, 15, 15, 1, 0, 0};
    return Player(id, id, name, race, cls, cfg, FormulaEngine::getInstance());
}

// =========================================================================
// TEST 1: COMPRA UNITARIA EXITOSA (Usando la Espada 4001 de tu TOML)
// =========================================================================
TEST(MerchantTest, Merchant_BuySingleWeaponSuccessfully) {
    ItemRegistry registry("../config/items.toml");
    Position merchantPos{5, 5};
    Merchant comerciante(1, merchantPos, registry);
    Player player = makeTestPlayer();

    // Configuramos precondiciones usando la API real de tu InventoryComponent
    player.addGold(500);

    // Intentamos comprar la "Espada"
    NpcCommandDTO cmd = createTestCommand(NpcCommandType::BUY, "2000");
    InteractionResult res = comerciante.handleCommand(player, cmd);

    // VALIDACIONES:
    // 0. Verificamos que la compra fue exitosa
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
    // 1. Se debitó el oro del inventario (500 - 100 de unitPrice base = 400)
    EXPECT_EQ(player.getGold(), 400u);

    // 2. El ítem 4001 ingresó efectivamente en el slot 0 de la mochila
    auto slotOpt = player.inspectSlot(0);
    ASSERT_TRUE(slotOpt.has_value());
    EXPECT_EQ(slotOpt->item_id, 2000u);
    EXPECT_EQ(slotOpt->amount, 1);
}

// =========================================================================
// TEST 2: MOCHILA LLENA EJECUTA ROLLBACK (No roba el oro del Player)
// =========================================================================
TEST(MerchantTest, Merchant_BuyWithFullInventoryDoesNotStealGold) {
    ItemRegistry registry("../config/items.toml");
    Merchant comerciante(1, {0, 0}, registry);
    Player player = makeTestPlayer();

    player.addGold(200);

    // Llenamos por completo la mochila del jugador usando IDs únicos
    // para evitar apilamiento automático del mismo ítem.
    uint8_t sizeMochila = player.getSize();
    for (uint8_t i = 0; i < sizeMochila; ++i) {
        player.addItem(1000u + i, 1);  // IDs distintos para ocupar cada slot
    }

    NpcCommandDTO cmd = createTestCommand(NpcCommandType::BUY, "2000");
    InteractionResult res = comerciante.handleCommand(player, cmd);

    // VALIDACIONES CRÍTICAS:
    // 0. Verificamos que la compra fue rechazada
    EXPECT_EQ(res.status, InteractionStatus::FAILURE);
    // 1. ¡Rollback exitoso! El oro debe permanecer en 200 de forma segura
    EXPECT_EQ(player.getGold(), 200u);
}

// =========================================================================
// TEST 3: FILTRO DE MAGIA RECHAZA BÁCULOS (Usando el Báculo Nudoso 6003)
// =========================================================================
TEST(MerchantTest, Merchant_RejectsMagicItemsBasedOnName) {
    ItemRegistry registry("../config/items.toml");
    Merchant comerciante(1, {0, 0}, registry);
    Player player = makeTestPlayer();

    player.addGold(300);

    NpcCommandDTO cmd = createTestCommand(NpcCommandType::BUY, "2022");  // Baculo nudoso
    InteractionResult res = comerciante.handleCommand(player, cmd);

    // 0. Verificamos que la compra fue rechazada por filtro de magia
    EXPECT_EQ(res.status, InteractionStatus::FAILURE);
    // 1. Verificamos que no se le haya descontado un solo centavo de oro
    EXPECT_EQ(player.getGold(), 300u);

    // 2. Verificamos que el inventario siga vacío (no se agregó el báculo)
    // (Asumiendo que arranca vacía con el helper makeTestPlayer)
    if (player.getSize() > 0) {
        auto slotOpt = player.inspectSlot(0);
        EXPECT_FALSE(slotOpt.has_value());
    }
}

// =========================================================================
// TEST 4: VENTA UNITARIA SUBE EL STOCK DEL MERCADER
// =========================================================================
TEST(MerchantTest, Merchant_SellItemIncrementsMerchantStock) {
    ItemRegistry registry("../config/items.toml");
    Merchant comerciante(1, {0, 0}, registry);
    Player player = makeTestPlayer();

    // Le damos una "Armadura de cuero" (ID: 1001) directo en la mochila
    player.addItem(1000, 1);
    EXPECT_EQ(player.getGold(), 0u);  // Arranca seco

    // Ejecutamos el comando de venta de la armadura
    NpcCommandDTO cmd = createTestCommand(NpcCommandType::SELL, "1000");
    InteractionResult res = comerciante.handleCommand(player, cmd);

    // VALIDACIONES:
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);

    auto slotOpt = player.inspectSlot(0);
    EXPECT_FALSE(slotOpt.has_value());

    // Traemos el precio base del .toml (150) y calculamos la mitad (75)
    uint32_t precioBase = registry.get_item(1000u)->getPrice();
    uint32_t oroEsperado = precioBase / 2;

    EXPECT_EQ(player.getGold(), oroEsperado);
}
