#include <gtest/gtest.h>

#include "../client/src/rendering/TallFlora.h"

TEST(TallFloraTest, TreesPalmsAndCactiOcclude) {
    EXPECT_TRUE(isTallFlora(10));
    EXPECT_TRUE(isTallFlora(41));
    EXPECT_TRUE(isTallFlora(136));
}

TEST(TallFloraTest, GroundBuildingsAndSignsDoNotOcclude) {
    EXPECT_FALSE(isTallFlora(0));
    EXPECT_FALSE(isTallFlora(107));
    EXPECT_FALSE(isTallFlora(161));
    EXPECT_FALSE(isTallFlora(200));
}
