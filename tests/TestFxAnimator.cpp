#include <gtest/gtest.h>

#include "FxAnimator.h"

TEST(FxAnimatorTest, FirstFrameAtStart) {
    EXPECT_EQ(fxFrameIndex(0, 60, 5), 0);
}

TEST(FxAnimatorTest, AdvancesWithTime) {
    EXPECT_EQ(fxFrameIndex(130, 60, 5), 2);
}

TEST(FxAnimatorTest, ReturnsNegativeWhenFinished) {
    EXPECT_EQ(fxFrameIndex(300, 60, 5), -1);
}

TEST(FxAnimatorTest, ZeroFrameCountIsFinished) {
    EXPECT_EQ(fxFrameIndex(0, 60, 0), -1);
}

TEST(FxAnimatorTest, ZeroDurationIsFinished) {
    EXPECT_EQ(fxFrameIndex(10, 0, 5), -1);
}

TEST(FxAnimatorTest, FrameRectSingleRow) {
    FrameRect rect = fxFrameRect(2, 32, 48, 4);
    EXPECT_EQ(rect.x, 64);
    EXPECT_EQ(rect.y, 0);
    EXPECT_EQ(rect.w, 32);
    EXPECT_EQ(rect.h, 48);
}

TEST(FxAnimatorTest, FrameRectWrapsToNextRow) {
    FrameRect rect = fxFrameRect(5, 32, 48, 4);
    EXPECT_EQ(rect.x, 32);
    EXPECT_EQ(rect.y, 48);
}
