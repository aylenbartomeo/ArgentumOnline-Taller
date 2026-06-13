#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../common/include/dto/CommandDTO.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

#include "Priest.h"
#include "World.h"

static NpcCommandDTO createPriestTestCommand(NpcCommandType type,
                                             const std::string& itemIdStr = "") {
    NpcCommandDTO dto;
    dto.type = type;
    dto.arg = itemIdStr;
    return dto;
}

static Player makePriestTestPlayer(uint32_t id = 1) {
    std::string name = "TestPlayer";
    RaceConfig race = {1.0f, 1.0f, 1.0f};
    CharacterClassConfig cls = {1.0f, 1.0f, 1.0f, false};
    PlayerConfig cfg = {15, 15, 15, 15, 1, 0, 0};
    InventoryConfig invCfg = {16, 0, 10000, 5000};

    // Invoca al constructor de test proveído en tu firma
    return Player(id, id, name, Race::HUMAN, CharacterClass::WARRIOR, race, cls, cfg, invCfg,
                  FormulaEngine::getInstance());
}

// =========================================================================
// TEST 1: EL SACERDOTE RESUCITA A UN FANTASMA (/resucitar)
// =========================================================================
TEST(PriestTest, Priest_ResurrectsDeadPlayerSuccessfully) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry);
    Player player = makePriestTestPlayer();

    // Gatillamos la muerte usando tu método sobreescrito de la interfaz
    player.handleDeath();
    ASSERT_TRUE(player.isDead());

    NpcCommandDTO cmd = createPriestTestCommand(NpcCommandType::RESPAWN);

    // CAPTURAMOS EL RESULTADO
    InteractionResult res = sacerdote.handleCommand(player, cmd);

    // VALIDACIONES DEL PROCESO:
    EXPECT_FALSE(player.isDead());
    EXPECT_GT(player.getHp(), 0u);

    // VALIDACIONES DEL NUEVO CONTRATO:
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
}

// =========================================================================
// TEST 2: EL SACERDOTE CURA VIDA Y MANÁ AL MÁXIMO (/curar)
// =========================================================================
TEST(PriestTest, Priest_HealsHpAndManaToMaximum) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry);
    Player player = makePriestTestPlayer();

    // Le hacemos daño para drenar estadísticas
    player.receiveDamage(5);
    player.consumeMana(10);

    ASSERT_LT(player.getHp(), player.getMaxHp());

    NpcCommandDTO cmd = createPriestTestCommand(NpcCommandType::HEAL);

    // CAPTURAMOS EL RESULTADO
    InteractionResult res = sacerdote.handleCommand(player, cmd);

    // VALIDACIONES DEL PROCESO:
    EXPECT_EQ(player.getHp(), player.getMaxHp());
    EXPECT_EQ(player.getMana(), player.getMaxMana());

    // VALIDACIONES DEL NUEVO CONTRATO:
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
}

// =========================================================================
// TEST 3: COMERCIO - COMPRA EXITOSA DE UN ÍTEM
// =========================================================================
TEST(PriestTest, Priest_AllowsBuyingItemsInStock) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry);
    sacerdote.initializeStock({{1001, 10}, {1002, 5}});
    Player player = makePriestTestPlayer();

    player.addGold(500);

    // Compra directa de la Armadura 1001 usando la firma 'dto.arg'
    NpcCommandDTO cmd = createPriestTestCommand(NpcCommandType::BUY, "1001");

    // CAPTURAMOS EL RESULTADO
    InteractionResult res = sacerdote.handleCommand(player, cmd);

    // VALIDACIONES DEL PROCESO:
    // Nota: Cambiar el "400u" si el precio en items.toml para el id 1001 es diferente de 100 de oro
    uint32_t expectedGold = 500u - registry.get_item(1001u)->getPrice();
    EXPECT_EQ(player.getGold(), expectedGold);

    auto slotOpt = player.inspectSlot(0);
    ASSERT_TRUE(slotOpt.has_value());
    EXPECT_EQ(slotOpt->item_id, 1001u);

    // VALIDACIONES DEL NUEVO CONTRATO:
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);
}

// =========================================================================
// TEST 4: UN FANTASMA NO PUEDE COMERCIAR
// =========================================================================
TEST(PriestTest, Priest_DeadPlayerCannotTrade) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry);
    Player player = makePriestTestPlayer();

    player.addGold(500);
    player.handleDeath();  // Lo convertimos en fantasma

    ASSERT_TRUE(player.isDead());

    NpcCommandDTO cmd = createPriestTestCommand(NpcCommandType::BUY, "1001");

    // CAPTURAMOS EL RESULTADO
    InteractionResult res = sacerdote.handleCommand(player, cmd);

    // VALIDACIONES DEL PROCESO:
    EXPECT_EQ(player.getGold(), 500u);  // No debe gastar oro

    if (player.getSize() > 0) {
        auto slotOpt = player.inspectSlot(0);
        EXPECT_FALSE(slotOpt.has_value());  // No debe recibir ítems
    }

    // VALIDACIONES DEL NUEVO CONTRATO:
    // pero falló la regla de negocio por estar muerto.
    EXPECT_EQ(res.status, InteractionStatus::FAILURE);
}

// =========================================================================
// TEST 5: EL SACERDOTE MUESTRA SU CATÁLOGO EXCLUSIVO DE VENTA
// =========================================================================
TEST(PriestTest, Priest_ListStockSuccessfully) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry);
    sacerdote.initializeStock({{1001, 10}, {1002, 5}});
    Player player = makePriestTestPlayer();

    // El jugador ejecuta el comando de listado ante el sacerdote
    NpcCommandDTO cmd = createPriestTestCommand(NpcCommandType::LIST, "");
    InteractionResult res = sacerdote.handleCommand(player, cmd);

    // VALIDACIONES:
    EXPECT_EQ(res.status, InteractionStatus::SUCCESS);

    // El mensaje debe estructurar el catálogo básico
    EXPECT_NE(res.msg.find("--- CATÁLOGO DISPONIBLE ---"), std::string::npos);

    // El Sacerdote inicia con ítems específicos en su stock (1001u y 1002u)
    EXPECT_NE(res.msg.find("[ID: 1001]"), std::string::npos);
    EXPECT_NE(res.msg.find("[ID: 1002]"), std::string::npos);
}
