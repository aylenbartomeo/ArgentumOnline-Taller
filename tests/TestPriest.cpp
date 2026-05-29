#include <string>
#include <vector>

#include <gtest/gtest.h>

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

    // Invoca al constructor de test proveído en tu firma
    return Player(id, id, name, race, cls, cfg, FormulaEngine::getInstance());
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

    sacerdote.handleCommand(player, cmd);

    // VALIDACIONES:
    EXPECT_FALSE(player.isDead());
    EXPECT_GT(player.getHp(), 0u);
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

    sacerdote.handleCommand(player, cmd);

    // VALIDACIONES:
    EXPECT_EQ(player.getHp(), player.getMaxHp());
    EXPECT_EQ(player.getMana(), player.getMaxMana());
}

// =========================================================================
// TEST 3: COMERCIO LIBRE - COMPRA EXITOSA DE UN ÍTEM
// =========================================================================
TEST(PriestTest, Priest_AllowsFreeTradeBuyingAnyItem) {
    ItemRegistry registry("../config/items.toml");
    Priest sacerdote(1, {0, 0}, registry);
    Player player = makePriestTestPlayer();

    player.addGold(500);

    // Compra directa de la Armadura 1001 usando la firma 'dto.arg'
    NpcCommandDTO cmd = createPriestTestCommand(NpcCommandType::BUY, "1001");

    sacerdote.handleCommand(player, cmd);

    // VALIDACIONES:
    EXPECT_EQ(player.getGold(), 400u);

    auto slotOpt = player.inspectSlot(0);
    ASSERT_TRUE(slotOpt.has_value());
    EXPECT_EQ(slotOpt->item_id, 1001u);
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

    NpcCommandDTO cmd = createPriestTestCommand(NpcCommandType::BUY, "4001");

    sacerdote.handleCommand(player, cmd);

    // VALIDACIONES:
    EXPECT_EQ(player.getGold(), 500u);  // No gasta

    if (player.getSize() > 0) {
        auto slotOpt = player.inspectSlot(0);
        EXPECT_FALSE(slotOpt.has_value());  // No recibe ítems
    }
}
