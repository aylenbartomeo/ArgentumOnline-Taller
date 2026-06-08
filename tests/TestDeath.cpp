#include <gtest/gtest.h>

#include "../client/src/animation/Death.h"

TEST(DeathTest, ZeroHpIsDead) { EXPECT_TRUE(isDead(0)); }

TEST(DeathTest, PositiveHpIsAlive) { EXPECT_FALSE(isDead(50)); }

TEST(DeathTest, NegativeHpIsDead) { EXPECT_TRUE(isDead(-5)); }

TEST(DeathTest, SkullFrameRectIsRow1Col4) {
    FrameRect rect = skullFrameRect();
    EXPECT_EQ(rect.x, 96);
    EXPECT_EQ(rect.y, 0);
    EXPECT_EQ(rect.w, 32);
    EXPECT_EQ(rect.h, 32);
}
