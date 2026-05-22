#include <gtest/gtest.h>
#include "model/components/StatsComponent.h"

TEST(StatsComponentTest, StatsCalculateCorrectlyFromConfig) {
    // Simulamos configs del archivo characters.toml
    RaceConfig race = {1.0f, 1.0f, 1.0f}; // Human
    CharacterClassConfig charClass = {0.7f, 1.5f, 1.5f, true}; // Mage
    PlayerConfig pBase = {15, 15, 15, 15, 1, 0, 0}; // Stats base
    
    StatsComponent stats(race, charClass, pBase);
    
    // Validamos que los maximos no sean 0 (basado en la logica del FormulaEngine)
    EXPECT_GT(stats.getMaxHp(), 0);
    EXPECT_EQ(stats.getHp(), stats.getMaxHp());
}