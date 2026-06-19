#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

#include "Merchant.h"
#include "TestHelpers.h"  // <--- Incluimos tu nuevo archivo unificado de helpers
#include "World.h"

// =========================================================================
// TEST 1: COMPRA UNITARIA EXITOSA (Usando la Espada 4001 de tu TOML)
// =========================================================================
TEST(MerchantTest, Merchant_BuySingleWeaponSuccessfully) {
    ItemRegistry registry("../config/items.toml");
    Position merchantPos{5, 5};
    Merchant comerciante(1, merchantPos, registry, {{2000u, 10}});

    // 🚀 Instanciación limpia y unificada con TestUtils
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    player.addGold(500);

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::BUY, "Espada");
    InteractionResult res = comerciante.handleCommand(player, cmd);

    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
    EXPECT_EQ(player.getGold(), 480u);

    // El ítem 2000u ingresó efectivamente en el slot 0 de la mochila
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
    Position merchantPos{5, 5};
    Merchant comerciante(1, merchantPos, registry, {{2000u, 10}});
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    player.addGold(200);

    // Llenamos por completo la mochila del jugador usando IDs únicos
    // para evitar apilamiento automático del mismo ítem.
    uint8_t sizeMochila = player.getSize();
    for (uint8_t i = 0; i < sizeMochila; ++i) {
        player.addItem(1000u + i, 1);  // IDs distintos para ocupar cada slot
    }

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::BUY, "Espada");
    InteractionResult res = comerciante.handleCommand(player, cmd);

    // VALIDACIONES CRÍTICAS:
    EXPECT_EQ(res.status, InteractionStatus::FAILURE);
    // ¡Rollback exitoso! El oro debe permanecer intacto de forma segura
    EXPECT_EQ(player.getGold(), 200u);
}

// =========================================================================
// TEST 3: FILTRO DE MAGIA RECHAZA BÁCULOS (Usando el Báculo Nudoso 6003)
// =========================================================================
TEST(MerchantTest, Merchant_RejectsMagicItemsBasedOnName) {
    ItemRegistry registry("../config/items.toml");
    Position merchantPos{5, 5};
    Merchant comerciante(1, merchantPos, registry, {{2000u, 10}});
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    player.addGold(300);

    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::BUY, "Baculo nudoso");
    InteractionResult res = comerciante.handleCommand(player, cmd);

    // Verificamos que la compra fue rechazada por filtro de magia
    EXPECT_EQ(res.status, InteractionStatus::FAILURE);
    // Verificamos que no se le haya descontado un solo centavo de oro
    EXPECT_EQ(player.getGold(), 300u);

    // Verificamos que el inventario siga vacío (no se agregó el báculo)
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
    Position merchantPos{5, 5};
    Merchant comerciante(1, merchantPos, registry, {{2000u, 10}});
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    // Le damos una "Armadura de cuero" (ID: 1000) directo en la mochila
    player.addItem(1000, 1);
    EXPECT_EQ(player.getGold(), 0u);  // Arranca sin oro

    // Ejecutamos el comando de venta de la armadura
    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::SELL, "Armadura de cuero");
    InteractionResult res = comerciante.handleCommand(player, cmd);

    // VALIDACIONES:
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);

    auto slotOpt = player.inspectSlot(0);
    EXPECT_FALSE(slotOpt.has_value());

    // Traemos el precio base del .toml y calculamos la mitad por venta de usados
    uint32_t precioBase = registry.get_item(1000u)->getPrice();
    uint32_t oroEsperado = precioBase / 2;

    EXPECT_EQ(player.getGold(), oroEsperado);
}

// =========================================================================
// TEST 5: EL MERCADER LISTA SU STOCK DISPONIBLE (CON PRECIOS Y CANTIDAD)
// =========================================================================
TEST(MerchantTest, Merchant_ListStockSuccessfully) {
    ItemRegistry registry("../config/items.toml");
    Position merchantPos{5, 5};
    Merchant comerciante(1, merchantPos, registry, {{2000u, 10}});
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    // El jugador ejecuta el comando para listar la tienda del mercader
    NpcCommandDTO cmd = TestUtils::createTestCommand(NpcCommandType::LIST, "");
    InteractionResult res = comerciante.handleCommand(player, cmd);

    // VALIDACIONES:
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);

    // Verificamos formato del catálogo expuesto en la red
    EXPECT_NE(res.msg.find("--- CATÁLOGO DISPONIBLE ---"), std::string::npos);
    EXPECT_NE(res.msg.find("Disp:"), std::string::npos);
    EXPECT_NE(res.msg.find("Compra:"), std::string::npos);

    // Verificamos que la espada del stock inicial esté mapeada en el texto string literal
    EXPECT_NE(res.msg.find("Espada"), std::string::npos);
}
