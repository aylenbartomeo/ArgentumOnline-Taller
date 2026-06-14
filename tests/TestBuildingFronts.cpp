#include <gtest/gtest.h>

#include "../client/src/rendering/BuildingFronts.h"

TEST(BuildingFrontsTest, ChurchAndBankHaveFrontBands) {
    EXPECT_EQ(buildingFrontTiles(201).value_or(-1), 6);
    EXPECT_EQ(buildingFrontTiles(202).value_or(-1), 4);
}

TEST(BuildingFrontsTest, NonBuildingsHaveNoFrontBand) {
    EXPECT_FALSE(buildingFrontTiles(0).has_value());
    EXPECT_FALSE(buildingFrontTiles(11).has_value());
    EXPECT_FALSE(buildingFrontTiles(204).has_value());
}

TEST(BuildingFrontsTest, RoofsDropOntoTheirBuildings) {
    EXPECT_EQ(roofDropPixels(204), 200);
    EXPECT_EQ(roofDropPixels(205), 194);
}

TEST(BuildingFrontsTest, NonRoofsHaveNoDrop) {
    EXPECT_EQ(roofDropPixels(0), 0);
    EXPECT_EQ(roofDropPixels(201), 0);
}
