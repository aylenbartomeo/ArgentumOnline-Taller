#include <gtest/gtest.h>

#include "model/components/RegenerationComponent.h"
#include "model/components/StateComponent.h"
#include "model/components/StatsComponent.h"

class RegenerationComponentTest: public ::testing::Test {
protected:
    RaceConfig humanRace{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f};
    CharacterClassConfig mageClass{0.7f, 1.5f, 3.0f, true};

    CharacterClassConfig warriorClass{1.8f, 0.0f, 0.0f, false};

    PlayerConfig base{15, 15, 15, 15, 5, 0, 0};

    static constexpr float ONE_SECOND = 1.0f;
    static constexpr float TEN_SECONDS = 10.0f;
};

// ============================================================================
// 1. FANTASMA — no recupera nada
// ============================================================================
TEST_F(RegenerationComponentTest, GhostDoesNotRecoverHpNorMana) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;

    stats.takeDamage(30);
    stats.consumeMana(30);
    state.die();

    uint16_t hpBefore = stats.getHp();
    uint16_t manaBefore = stats.getMana();

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(TEN_SECONDS);

    EXPECT_EQ(stats.getHp(), hpBefore);
    EXPECT_EQ(stats.getMana(), manaBefore);
}

// ============================================================================
// 2. TICK CON TIEMPO <= 0 — no hace nada
// ============================================================================
TEST_F(RegenerationComponentTest, ZeroDeltaDoesNothing) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.takeDamage(10);
    stats.consumeMana(10);

    uint16_t hpBefore = stats.getHp();
    uint16_t manaBefore = stats.getMana();

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(0.0f);

    EXPECT_EQ(stats.getHp(), hpBefore);
    EXPECT_EQ(stats.getMana(), manaBefore);
}

TEST_F(RegenerationComponentTest, NegativeDeltaDoesNothing) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.takeDamage(10);

    uint16_t hpBefore = stats.getHp();

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(-5.0f);

    EXPECT_EQ(stats.getHp(), hpBefore);
}

// ============================================================================
// 3. RECUPERACIÓN PASIVA DE VIDA (estado normal)
// ============================================================================
TEST_F(RegenerationComponentTest, PassiveHpRecoveryInNormalState) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.takeDamage(20);

    uint16_t hpBefore = stats.getHp();

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(ONE_SECOND);

    // recover = 2.0 * 1.0 = 2
    EXPECT_EQ(stats.getHp(), hpBefore + 2);
}

TEST_F(RegenerationComponentTest, PassiveHpRecoveryScalesWithTime) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.takeDamage(50);

    uint16_t hpBefore = stats.getHp();

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(TEN_SECONDS);

    // recover = 2.0 * 10.0 = 20
    EXPECT_EQ(stats.getHp(), hpBefore + 20);
}

TEST_F(RegenerationComponentTest, HpRecoveryDoesNotExceedMaxHp) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.takeDamage(1);  // solo 1 punto de daño

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(9999.0f);  // tiempo enorme

    EXPECT_EQ(stats.getHp(), stats.getMaxHp());
}

// ============================================================================
// 4. RECUPERACIÓN PASIVA DE MANÁ (estado normal, no meditando)
// ============================================================================
TEST_F(RegenerationComponentTest, PassiveManaRecoveryInNormalState) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.consumeMana(20);

    uint16_t manaBefore = stats.getMana();

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(ONE_SECOND);

    // recover = 2.0 * 1.0 = 2
    EXPECT_EQ(stats.getMana(), manaBefore + 2);
}

TEST_F(RegenerationComponentTest, ManaRecoveryDoesNotExceedMaxMana) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.consumeMana(1);

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(9999.0f);

    EXPECT_EQ(stats.getMana(), stats.getMaxMana());
}

// ============================================================================
// 5. MEDITACIÓN
// ============================================================================
TEST_F(RegenerationComponentTest, MeditationRecoversManaFaster) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.consumeMana(50);
    state.startMeditating();
    ASSERT_TRUE(state.isMeditating());

    uint16_t manaBefore = stats.getMana();

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(ONE_SECOND);

    EXPECT_EQ(stats.getMana(), manaBefore + 45);
}

// ============================================================================
// 6. AUTO-SALIDA DE MEDITACIÓN al llenarse el maná
// ============================================================================
TEST_F(RegenerationComponentTest, MeditationStopsAutomaticallyWhenManaFull) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.consumeMana(2);
    state.startMeditating();
    ASSERT_TRUE(state.isMeditating());

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(ONE_SECOND);  // 45 de recover >> 2 de déficit → se llena

    EXPECT_EQ(stats.getMana(), stats.getMaxMana());
    EXPECT_FALSE(state.isMeditating());  // salió de meditación
}

TEST_F(RegenerationComponentTest, MeditationContinuesIfManaNotFull) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.consumeMana(stats.getMaxMana());
    state.startMeditating();

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(ONE_SECOND);

    EXPECT_TRUE(state.isMeditating());
}

// ============================================================================
// 7. GUERRERO — nunca recupera maná
// ============================================================================
TEST_F(RegenerationComponentTest, WarriorDoesNotRecoverMana) {
    StatsComponent stats(humanRace, warriorClass, base);
    StateComponent state;

    ASSERT_EQ(stats.getMana(), 0);

    RegenerationComponent regen(stats, state, humanRace, warriorClass);
    regen.tick(TEN_SECONDS);

    EXPECT_EQ(stats.getMana(), 0);
}

TEST_F(RegenerationComponentTest, WarriorStillRecoversHp) {
    StatsComponent stats(humanRace, warriorClass, base);
    StateComponent state;
    stats.takeDamage(20);

    uint16_t hpBefore = stats.getHp();

    RegenerationComponent regen(stats, state, humanRace, warriorClass);
    regen.tick(ONE_SECOND);

    EXPECT_GT(stats.getHp(), hpBefore);
}

// ============================================================================
// 8. VIDA TAMBIÉN SE RECUPERA DURANTE MEDITACIÓN
// ============================================================================
TEST_F(RegenerationComponentTest, HpAlsoRecoversDuringMeditation) {
    StatsComponent stats(humanRace, mageClass, base);
    StateComponent state;
    stats.takeDamage(10);
    stats.consumeMana(10);
    state.startMeditating();

    uint16_t hpBefore = stats.getHp();

    RegenerationComponent regen(stats, state, humanRace, mageClass);
    regen.tick(ONE_SECOND);

    EXPECT_GT(stats.getHp(), hpBefore);
}
