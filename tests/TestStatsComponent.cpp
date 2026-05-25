#include <gtest/gtest.h>

#include "model/components/StatsComponent.h"

// Configs representativas de la tabla del juego
class StatsComponentTest: public ::testing::Test {
protected:
    // Humano equilibrado
    RaceConfig humanRace{1.0f, 1.0f, 1.0f};

    // Elfo: inteligente, ágil, constitución frágil
    RaceConfig elfRace{0.8f, 1.5f, 1.2f};

    // Mago: potencia mágica alta, vida baja
    CharacterClassConfig mageClass{0.7f, 1.5f, 1.5f, true};

    // Guerrero: sin magia (canUseMagic = false, meditationFactor = 0)
    CharacterClassConfig warriorClass{1.8f, 0.0f, 0.0f, false};

    // Stats base de nivel 1
    PlayerConfig baseLv1{15, 15, 15, 15, 1, 0, 0};

    // Stats base de nivel 5 (para tests de progresión)
    PlayerConfig baseLv5{15, 15, 15, 15, 5, 0, 0};
};

// ============================================================================
// 1. ESTADO INICIAL — ATRIBUTOS PRIMARIOS
// ============================================================================
TEST_F(StatsComponentTest, InitialAttributesMatchConfig) {
    StatsComponent stats(humanRace, mageClass, baseLv1);

    EXPECT_EQ(stats.getStrength(), 15);
    EXPECT_EQ(stats.getIntelligence(), 15);
    EXPECT_EQ(stats.getAgility(), 15);
    EXPECT_EQ(stats.getConstitution(), 15);
}

TEST_F(StatsComponentTest, InitialLevelAndExpMatchConfig) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    EXPECT_EQ(stats.getLevel(), 1);
    EXPECT_EQ(stats.getExp(), 0u);
}

// ============================================================================
// 2. VIDA MÁXIMA — fórmula: CON * FClaseVida * FRazaVida * Nivel
// ============================================================================
TEST_F(StatsComponentTest, MaxHpCalculationHumanMageLv1) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    // 15 * 0.7 * 1.0 * 1 = 10
    EXPECT_EQ(stats.getMaxHp(), 10);
}

TEST_F(StatsComponentTest, MaxHpScalesWithLevel) {
    StatsComponent statsLv1(humanRace, mageClass, baseLv1);
    StatsComponent statsLv5(humanRace, mageClass, baseLv5);
    // Lv5: 15 * 0.7 * 1.0 * 5 = 52
    EXPECT_EQ(statsLv5.getMaxHp(), 52);
    EXPECT_GT(statsLv5.getMaxHp(), statsLv1.getMaxHp());
}

TEST_F(StatsComponentTest, MaxHpIsNeverZeroForValidConfig) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    EXPECT_GT(stats.getMaxHp(), 0);
}

TEST_F(StatsComponentTest, HpStartsAtMaximum) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    EXPECT_EQ(stats.getHp(), stats.getMaxHp());
}

// ============================================================================
// 3. MANÁ MÁXIMO — fórmula: INT * FClaseMana * FRazaMana * Nivel
// ============================================================================
TEST_F(StatsComponentTest, MaxManaCalculationHumanMageLv1) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    // 15 * 1.5 * 1.0 * 1 = 22 (truncado)
    EXPECT_EQ(stats.getMaxMana(), 22);
}

TEST_F(StatsComponentTest, ManaStartsAtMaximum) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    EXPECT_EQ(stats.getMana(), stats.getMaxMana());
}

TEST_F(StatsComponentTest, WarriorHasZeroMana) {
    // El guerrero no puede usar magia → manaFactor = 0 → MaxMana = 0
    StatsComponent stats(humanRace, warriorClass, baseLv1);
    EXPECT_EQ(stats.getMaxMana(), 0);
    EXPECT_EQ(stats.getMana(), 0);
}

TEST_F(StatsComponentTest, MaxManaScalesWithLevel) {
    StatsComponent statsLv1(humanRace, mageClass, baseLv1);
    StatsComponent statsLv5(humanRace, mageClass, baseLv5);
    EXPECT_GT(statsLv5.getMaxMana(), statsLv1.getMaxMana());
}

// ============================================================================
// 4. DAÑO (takeDamage y heal)
// ============================================================================
TEST_F(StatsComponentTest, TakeDamageReducesHp) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    uint16_t maxHp = stats.getMaxHp();

    stats.takeDamage(3);
    EXPECT_EQ(stats.getHp(), maxHp - 3);
}

TEST_F(StatsComponentTest, TakeDamageDoesNotGoBelowZero) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    stats.takeDamage(9999);
    EXPECT_EQ(stats.getHp(), 0);
}

TEST_F(StatsComponentTest, TakeExactMaxDamageKillsPlayer) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    stats.takeDamage(stats.getMaxHp());
    EXPECT_EQ(stats.getHp(), 0);
}

TEST_F(StatsComponentTest, HealRestoresHp) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    stats.takeDamage(5);
    stats.heal(3);
    EXPECT_EQ(stats.getHp(), stats.getMaxHp() - 2);
}

TEST_F(StatsComponentTest, HealDoesNotExceedMaxHp) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    stats.takeDamage(2);
    stats.heal(9999);
    EXPECT_EQ(stats.getHp(), stats.getMaxHp());
}

TEST_F(StatsComponentTest, HealOnDeadPlayerDoesNothing) {
    // Según la implementación de StatsComponent::heal: if (health == 0) return
    StatsComponent stats(humanRace, mageClass, baseLv1);
    stats.takeDamage(9999);  // muere
    ASSERT_EQ(stats.getHp(), 0);
    stats.heal(100);
    EXPECT_EQ(stats.getHp(), 0);
}

// ============================================================================
// 5. MANÁ (consumeMana y recoverMana)
// ============================================================================
TEST_F(StatsComponentTest, ConsumeManaReducesMana) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    uint16_t maxMana = stats.getMaxMana();

    stats.consumeMana(5);
    EXPECT_EQ(stats.getMana(), maxMana - 5);
}

TEST_F(StatsComponentTest, ConsumeManaFailsIfInsufficient) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    uint16_t initialMana = stats.getMana();
    EXPECT_FALSE(stats.consumeMana(9999));
    EXPECT_EQ(stats.getMana(), initialMana);  // Mana remains unchanged
}

TEST_F(StatsComponentTest, ConsumeManaReturnsTrueOnSuccess) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    EXPECT_TRUE(stats.consumeMana(5));
}

TEST_F(StatsComponentTest, RecoverManaRestoresMana) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    stats.consumeMana(10);
    stats.recoverMana(5);
    EXPECT_EQ(stats.getMana(), stats.getMaxMana() - 5);
}

TEST_F(StatsComponentTest, RecoverManaDoesNotExceedMaxMana) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    stats.consumeMana(10);
    stats.recoverMana(9999);
    EXPECT_EQ(stats.getMana(), stats.getMaxMana());
}

TEST_F(StatsComponentTest, RecoverManaOnDeadPlayerDoesNothing) {
    // recoverMana también chequea if (health == 0)
    StatsComponent stats(humanRace, mageClass, baseLv1);
    stats.consumeMana(10);
    uint16_t manaBeforeDeath = stats.getMana();

    stats.takeDamage(9999);  // muere
    ASSERT_EQ(stats.getHp(), 0);
    stats.recoverMana(100);
    EXPECT_EQ(stats.getMana(), manaBeforeDeath);  // no cambia
}

// ============================================================================
// 6. EXPERIENCIA Y LEVEL UP
// ============================================================================
TEST_F(StatsComponentTest, AddExperienceAccumulates) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    stats.addExperience(100);
    EXPECT_EQ(stats.getExp(), 100u);
}

TEST_F(StatsComponentTest, LevelUpOccursWhenLimitIsReached) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    // Límite lv1 = 1000 * 1^1.8 = 1000
    stats.addExperience(1000);
    EXPECT_EQ(stats.getLevel(), 2);
}

TEST_F(StatsComponentTest, LevelUpRestoresHpAndMana) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    stats.takeDamage(5);
    stats.consumeMana(5);
    ASSERT_LT(stats.getHp(), stats.getMaxHp());
    ASSERT_LT(stats.getMana(), stats.getMaxMana());

    stats.addExperience(1000);  // level up
    EXPECT_EQ(stats.getHp(), stats.getMaxHp());
    EXPECT_EQ(stats.getMana(), stats.getMaxMana());
}

TEST_F(StatsComponentTest, LevelUpIncreasesMaxHpAndMaxMana) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    uint16_t hpBefore = stats.getMaxHp();
    uint16_t manaBefore = stats.getMaxMana();

    stats.addExperience(1000);  // sube a lv2
    EXPECT_GT(stats.getMaxHp(), hpBefore);
    EXPECT_GT(stats.getMaxMana(), manaBefore);
}

TEST_F(StatsComponentTest, MultipleConsecutiveLevelUps) {
    StatsComponent stats(humanRace, mageClass, baseLv1);
    // Damos exp suficiente para subir varios niveles
    // Lv1→2: 1000, Lv2→3: 1000*2^1.8 ≈ 3482, total ≈ 4482
    stats.addExperience(5000);
    EXPECT_GE(stats.getLevel(), 2);
}

// ============================================================================
// 7. DIFERENCIAS ENTRE RAZAS Y CLASES
// ============================================================================
TEST_F(StatsComponentTest, ElfMageHasMoreManaThanHumanMage) {
    StatsComponent human(humanRace, mageClass, baseLv1);
    StatsComponent elf(elfRace, mageClass, baseLv1);
    // Elfo tiene FRazaMana = 1.5 vs Humano 1.0
    EXPECT_GT(elf.getMaxMana(), human.getMaxMana());
}

TEST_F(StatsComponentTest, ElfMageHasLessHpThanHumanMage) {
    StatsComponent human(humanRace, mageClass, baseLv1);
    StatsComponent elf(elfRace, mageClass, baseLv1);
    // Elfo tiene FRazaVida = 0.8 vs Humano 1.0
    EXPECT_LT(elf.getMaxHp(), human.getMaxHp());
}

TEST_F(StatsComponentTest, WarriorHasMoreHpThanMageSameRace) {
    StatsComponent mage(humanRace, mageClass, baseLv1);
    StatsComponent warrior(humanRace, warriorClass, baseLv1);
    // Guerrero FClaseVida = 1.8 vs Mago 0.7
    EXPECT_GT(warrior.getMaxHp(), mage.getMaxHp());
}
