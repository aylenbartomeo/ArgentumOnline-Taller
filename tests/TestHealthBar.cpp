#include <gtest/gtest.h>

#include "../client/src/ui/HealthBar.h"

TEST(HealthBarTest, FullHpIsNotVisible) {
    HealthBarLayout layout = computeHealthBar(100, 100, 0, 0, 32);
    EXPECT_FALSE(layout.visible);
}

TEST(HealthBarTest, MaxHpZeroIsNotVisible) {
    HealthBarLayout layout = computeHealthBar(0, 0, 0, 0, 32);
    EXPECT_FALSE(layout.visible);
}

TEST(HealthBarTest, DamagedIsVisible) {
    HealthBarLayout layout = computeHealthBar(70, 100, 0, 0, 32);
    EXPECT_TRUE(layout.visible);
}

TEST(HealthBarTest, HalfHpFillsHalfTileWidth) {
    HealthBarLayout layout = computeHealthBar(50, 100, 0, 0, 32);
    EXPECT_TRUE(layout.visible);
    EXPECT_EQ(layout.fill.w, 16);
}

TEST(HealthBarTest, ZeroHpIsVisibleWithEmptyFill) {
    HealthBarLayout layout = computeHealthBar(0, 100, 0, 0, 32);
    EXPECT_TRUE(layout.visible);
    EXPECT_EQ(layout.fill.w, 0);
}

TEST(HealthBarTest, OverMaxHpIsTreatedAsFullAndNotVisible) {
    HealthBarLayout layout = computeHealthBar(150, 100, 0, 0, 32);
    EXPECT_FALSE(layout.visible);
}

TEST(HealthBarTest, BackgroundSpansTileWidthAtEntityX) {
    HealthBarLayout layout = computeHealthBar(50, 100, 100, 200, 32);
    EXPECT_EQ(layout.background.x, 100);
    EXPECT_EQ(layout.background.w, 32);
}

TEST(HealthBarTest, BarSitsAboveEntity) {
    HealthBarLayout layout = computeHealthBar(50, 100, 100, 200, 32);
    EXPECT_LT(layout.background.y, 200);
}

TEST(HealthBarTest, FillSharesBackgroundPositionAndHeight) {
    HealthBarLayout layout = computeHealthBar(50, 100, 100, 200, 32);
    EXPECT_EQ(layout.fill.x, layout.background.x);
    EXPECT_EQ(layout.fill.y, layout.background.y);
    EXPECT_EQ(layout.fill.h, layout.background.h);
}
