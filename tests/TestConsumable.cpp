#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "model/entities/Player.h"
#include "model/items/Consumable.h"

#include "TestHelpers.h"

// =========================================================================
// TEST 1: POCIÓN DE VIDA
// =========================================================================
TEST(ConsumableTest, Consumable_HealthPotionRestoresHp) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    uint16_t maxHp = player.getMaxHp();

    // Configurar poción de vida que cura 20 HP
    Consumable healthPotion(101, "Pocion Roja", 50, ConsumableType::HEALTH, 0, 20);

    // Caso 1: El jugador ya tiene vida al máximo, la poción no debería hacer efecto y retornar
    // false false
    EXPECT_FALSE(healthPotion.use(player));
    EXPECT_EQ(player.getHp(), maxHp);

    // Caso 2: El jugador tiene menos de la vida máxima (pero sigue vivo)
    player.receiveDamage(10);
    ASSERT_EQ(player.getHp(), maxHp - 10);

    // Curó 10 (se limitó a la vida máxima)
    EXPECT_TRUE(healthPotion.use(player));
    EXPECT_EQ(player.getHp(), maxHp);

    // Caso 3: El jugador tiene poca vida y se cura parcialmente
    player.receiveDamage(12);
    ASSERT_EQ(player.getHp(), maxHp - 12);

    // Configuramos otra poción que cura 5 HP
    Consumable smallHealthPotion(105, "Pocion Chica", 20, ConsumableType::HEALTH, 0, 5);
    EXPECT_TRUE(smallHealthPotion.use(player));
    EXPECT_EQ(player.getHp(), maxHp - 7);  // Curó 5 HP exactos
}

// =========================================================================
// TEST 2: POCIÓN DE MANÁ (MANA POTION)
// =========================================================================
TEST(ConsumableTest, Consumable_ManaPotionRestoresMana) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    uint16_t maxMana = player.getMaxMana();

    // Configurar poción de maná que restaura 30 de maná
    Consumable manaPotion(102, "Pocion Azul", 50, ConsumableType::MANA, 0, 30);

    // Caso 1: El jugador ya tiene maná al máximo, la poción no debe hacer efecto y retornar false
    EXPECT_FALSE(manaPotion.use(player));
    EXPECT_EQ(player.getMana(), maxMana);

    // Caso 2: El jugador consume maná y se restaura completamente (limitado a max)
    player.consumeMana(10);
    ASSERT_EQ(player.getMana(), maxMana - 10);

    EXPECT_TRUE(manaPotion.use(player));
    EXPECT_EQ(player.getMana(), maxMana);

    // Caso 3: El jugador consume maná y se restaura parcialmente
    player.consumeMana(12);
    ASSERT_EQ(player.getMana(), maxMana - 12);

    // Configuramos otra poción que restaura 5 de maná
    Consumable smallManaPotion(106, "Pocion Azul Chica", 20, ConsumableType::MANA, 0, 5);
    EXPECT_TRUE(smallManaPotion.use(player));
    EXPECT_EQ(player.getMana(), maxMana - 7);  // Restauró 5 exactos
}

// =========================================================================
// TEST 3: ELIXIR DE FUERZA (STRENGTH BOOST)
// =========================================================================
TEST(ConsumableTest, Consumable_StrengthBoostElixir) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    uint8_t baseStrength = player.getStrength();

    Consumable strElixir(103, "Elixir de Fuerza", 150, ConsumableType::BOOST_STR, 5000, 5);

    EXPECT_TRUE(strElixir.use(player));
    EXPECT_EQ(player.getStrength(), baseStrength + 5);
}

// =========================================================================
// TEST 4: ELIXIR DE AGILIDAD
// =========================================================================
TEST(ConsumableTest, Consumable_AgilityBoostElixir) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    uint8_t baseAgility = player.getAgility();

    // Configurar elixir que da +8 de Agilidad por 3000ms
    Consumable agiElixir(104, "Elixir de Agilidad", 150, ConsumableType::BOOST_AGI, 3000, 8);

    EXPECT_TRUE(agiElixir.use(player));
    EXPECT_EQ(player.getAgility(), baseAgility + 8);
}

// =========================================================================
// TEST 5: USO EN JUGADOR MUERTO
// =========================================================================
TEST(ConsumableTest, Consumable_UseWhenDeadFails) {
    Player player = TestUtils::makeTestPlayer(1, "TestPlayer");
    player.receiveDamage(player.getMaxHp());
    ASSERT_TRUE(player.isDead());

    Consumable healthPotion(101, "Pocion Roja", 50, ConsumableType::HEALTH, 0, 20);
    Consumable strElixir(103, "Elixir de Fuerza", 150, ConsumableType::BOOST_STR, 5000, 5);

    EXPECT_FALSE(healthPotion.use(player));
    EXPECT_EQ(player.getHp(), 0u);

    EXPECT_FALSE(strElixir.use(player));
}
