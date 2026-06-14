#include <gtest/gtest.h>

#include "../client/src/rendering/IndoorRegions.h"

TEST(IndoorRegionsTest, SeparatesDisconnectedRegions) {
    std::vector<std::vector<int>> indoor = {
            {1, 1, 0, 0, 1},
            {1, 0, 0, 0, 1},
            {0, 0, 0, 0, 0},
    };
    IndoorRegions r(indoor);
    EXPECT_GE(r.regionAt(0, 0), 0);
    EXPECT_EQ(r.regionAt(0, 0), r.regionAt(1, 0));
    EXPECT_NE(r.regionAt(0, 0), r.regionAt(4, 0));
    EXPECT_EQ(r.regionAt(2, 0), -1);
}

TEST(IndoorRegionsTest, HidesRoofsOfPlayersRegionOnly) {
    std::vector<std::vector<int>> indoor = {
            {1, 1, 0, 0, 1},
            {1, 0, 0, 0, 1},
    };
    IndoorRegions r(indoor);
    EXPECT_TRUE(r.roofHidden(1, 0, 0, 0));
    EXPECT_FALSE(r.roofHidden(4, 0, 0, 0));
    EXPECT_FALSE(r.roofHidden(0, 0, 2, 0));
}
