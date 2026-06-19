#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "model/components/StatsComponent.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

#include "TestHelpers.h"

// =========================================================================
// TEST 1: INICIALIZACIÓN DE ATRIBUTOS BÁSICOS
// =========================================================================
TEST(PlayerTest, Player_Initialization) {
    Player player = TestUtils::makeTestPlayer(42, "TestPlayer");

    EXPECT_EQ(player.getId(), 42u);
    EXPECT_EQ(player.getDbId(), 42u);
    EXPECT_EQ(player.getName(), "TestPlayer");
    EXPECT_EQ(player.getGold(), 0u);
    EXPECT_FALSE(player.isDead());
    EXPECT_EQ(player.getHp(), player.getMaxHp());
}

// =========================================================================
// TEST 2: RECEPCIÓN DE DAÑO Y MUERTE
// =========================================================================
TEST(PlayerTest, Player_DamageAndDeath) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    uint16_t initialHp = player.getHp();

    // Daño menor que la vida
    player.receiveDamage(initialHp / 2);
    EXPECT_EQ(player.getHp(), initialHp - (initialHp / 2));
    EXPECT_FALSE(player.isDead());

    // Daño que supera la vida restante (provoca muerte)
    player.receiveDamage(initialHp);
    EXPECT_TRUE(player.isDead());
    EXPECT_EQ(player.getHp(), 0u);
}

// =========================================================================
// TEST 3: RESURRECCIÓN
// =========================================================================
TEST(PlayerTest, Player_Resurrection) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    player.receiveDamage(player.getMaxHp());
    ASSERT_TRUE(player.isDead());

    player.resurrect();
    EXPECT_FALSE(player.isDead());
    EXPECT_EQ(player.getHp(), player.getMaxHp());
}

// =========================================================================
// TEST 4: INVENTARIO Y ORO
// =========================================================================
TEST(PlayerTest, Player_InventoryAndGoldOperations) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    // Adición de oro (mayor al límite seguro de 5000)
    EXPECT_TRUE(player.addGold(6000));
    EXPECT_EQ(player.getGold(), 6000u);

    // Remoción de oro
    EXPECT_TRUE(player.removeGold(400));
    EXPECT_EQ(player.getGold(), 5600u);

    // El exceso a tirar es 5600 - 5000 = 600.
    uint32_t droppedGold = player.dropExcessGold();
    EXPECT_EQ(droppedGold, 600u);
    EXPECT_EQ(player.getGold(), 5000u);

    // Añadir items directamente al inventario (sin ItemRegistry)
    EXPECT_TRUE(player.addItem(100, 5));
    auto slotOpt = player.inspectSlot(0);
    ASSERT_TRUE(slotOpt.has_value());
    EXPECT_EQ(slotOpt->item_id, 100u);
    EXPECT_EQ(slotOpt->amount, 5);

    // Remover item
    uint16_t removed = player.removeItem(0, 2);
    EXPECT_EQ(removed, 2);
    slotOpt = player.inspectSlot(0);
    ASSERT_TRUE(slotOpt.has_value());
    EXPECT_EQ(slotOpt->amount, 3);
}

// =========================================================================
// TEST 5: SIMULACIÓN DE MOVIMIENTO
// =========================================================================
TEST(PlayerTest, Player_Movement) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    Position startPos = player.getPosition();

    // Intentar moverse en una dirección
    Position nextPos = player.tryMove(Movement::RIGHT);
    EXPECT_EQ(nextPos.x, startPos.x + 1);
    EXPECT_EQ(nextPos.y, startPos.y);

    // Verificar que la posición del jugador no ha cambiado
    EXPECT_EQ(player.getPosition().x, startPos.x);

    // Establecer la nueva posición
    player.setPosition(nextPos);
    EXPECT_EQ(player.getPosition().x, startPos.x + 1);
}

// =========================================================================
// TEST 6: MANÁ
// =========================================================================
TEST(PlayerTest, Player_ManaUsage) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    uint16_t maxMana = player.getMaxMana();

    // Consumir maná disponible
    EXPECT_TRUE(player.consumeMana(maxMana / 2));
    EXPECT_EQ(player.getMana(), maxMana - (maxMana / 2));

    // Intentar consumir más maná del disponible
    EXPECT_FALSE(player.consumeMana(maxMana));
    EXPECT_EQ(player.getMana(), maxMana - (maxMana / 2));  // No debe alterarse

    // Restaurar maná
    player.restoreMana();
    EXPECT_EQ(player.getMana(), maxMana);
}

// =========================================================================
// TEST 7: REGLAS DE COMBATE (NIVEL / PVP / NEWBIE)
// =========================================================================
TEST(PlayerTest, Player_CombatRules) {
    Player newbie = TestUtils::makeTestPlayer(1, "Newbie");   // Nivel 1
    Player victim1 = TestUtils::makeTestPlayer(2, "Victim");  // Nivel 1

    // Regla 1: Un novato (newbie, nivel <= 12) no puede entrar en combate con otro jugador
    EXPECT_FALSE(newbie.canEngageInCombatWith(victim1));
    EXPECT_FALSE(victim1.canEngageInCombatWith(newbie));

    // Elevamos el nivel de los jugadores para que dejen de ser newbies
    newbie.addExperience(1000000);
    victim1.addExperience(1000000);

    ASSERT_GT(newbie.getLevel(), 12);
    ASSERT_GT(victim1.getLevel(), 12);

    // Ahora que no son newbies, deberían poder combatir si tienen diferencia de nivel <= 10
    EXPECT_TRUE(newbie.canEngageInCombatWith(victim1));

    // Creamos un tercer jugador de nivel alto para verificar la diferencia de nivel
    Player highLevel = TestUtils::makeTestPlayer(3, "HighLevel");
    highLevel.addExperience(10000000);

    // Verificar que la diferencia de nivel es > 10 y por lo tanto no pueden combatir
    ASSERT_GT(highLevel.getLevel() - victim1.getLevel(), 10);
    EXPECT_FALSE(victim1.canEngageInCombatWith(highLevel));
    EXPECT_FALSE(highLevel.canEngageInCombatWith(victim1));
}

// =========================================================================
// TEST 8: PÉRDIDA DE EXPERIENCIA POR MUERTE Y PROTECCIÓN DE NIVEL
// =========================================================================
TEST(PlayerTest, Player_ExperienceLossOnDeathAndFloorProtection) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");

    player.addExperience(10000);
    uint8_t levelBeforeDeath = player.getLevel();
    uint32_t expBeforeDeath = player.getExp();

    ASSERT_GT(levelBeforeDeath, 1u);

    player.receiveDamage(player.getHp());
    ASSERT_TRUE(player.isDead());

    EXPECT_LT(player.getExp(), expBeforeDeath);
    EXPECT_EQ(player.getLevel(), levelBeforeDeath);

    // Muertes consecutivas para buscar el "Downgrade" de nivel
    for (int i = 0; i < 5; ++i) {
        player.resurrect();
        player.receiveDamage(player.getHp());
    }

    EXPECT_EQ(player.getLevel(), levelBeforeDeath);

    // La experiencia debió estancarse exactamente en el piso requerido para su nivel actual.
    uint32_t expectedFloor =
            FormulaEngine::getInstance().calculateLevelUpLimit(levelBeforeDeath - 1);
    EXPECT_EQ(player.getExp(), expectedFloor);
}
