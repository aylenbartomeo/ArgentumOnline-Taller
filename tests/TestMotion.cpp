#include <gtest/gtest.h>

#include "../client/src/animation/Motion.h"

TEST(MotionTest, PartialAdvanceMovesProportionally) {
    Vec2 result = stepToward({0.0f, 0.0f}, {10.0f, 0.0f}, 0.005f, 100, 100.0f);
    EXPECT_NEAR(result.x, 0.5f, 0.0001f);
    EXPECT_NEAR(result.y, 0.0f, 0.0001f);
}

TEST(MotionTest, ArrivesExactlyWithoutOvershoot) {
    Vec2 result = stepToward({0.0f, 0.0f}, {1.0f, 0.0f}, 0.005f, 1000, 100.0f);
    EXPECT_FLOAT_EQ(result.x, 1.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
}

TEST(MotionTest, SnapsWhenJumpExceedsSnapDist) {
    Vec2 result = stepToward({0.0f, 0.0f}, {10.0f, 0.0f}, 0.005f, 100, 3.0f);
    EXPECT_FLOAT_EQ(result.x, 10.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
}

TEST(MotionTest, ZeroDtDoesNotMove) {
    Vec2 result = stepToward({2.0f, 3.0f}, {9.0f, 9.0f}, 0.005f, 0, 100.0f);
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
}

TEST(MotionTest, AlreadyAtTargetDoesNotMove) {
    Vec2 result = stepToward({5.0f, 5.0f}, {5.0f, 5.0f}, 0.005f, 100, 100.0f);
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 5.0f);
}

TEST(MotionTest, DiagonalAdvancesBothAxes) {
    Vec2 result = stepToward({0.0f, 0.0f}, {3.0f, 4.0f}, 0.005f, 200, 100.0f);
    EXPECT_NEAR(result.x, 0.6f, 0.0001f);
    EXPECT_NEAR(result.y, 0.8f, 0.0001f);
}
