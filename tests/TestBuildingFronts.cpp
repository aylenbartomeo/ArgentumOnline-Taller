#include <gtest/gtest.h>

#include "../client/src/rendering/BuildingFronts.h"

TEST(BuildingFrontsTest, ChurchAndBankHaveFrontBands) {
    EXPECT_TRUE(buildingFrontTiles(201).has_value());
    EXPECT_TRUE(buildingFrontTiles(202).has_value());
    EXPECT_GT(buildingFrontTiles(201).value_or(0), 0);
    EXPECT_GT(buildingFrontTiles(202).value_or(0), 0);
}

TEST(BuildingFrontsTest, NonBuildingsHaveNoFrontBand) {
    EXPECT_FALSE(buildingFrontTiles(0).has_value());
    EXPECT_FALSE(buildingFrontTiles(11).has_value());
    EXPECT_FALSE(buildingFrontTiles(204).has_value());
}

TEST(BuildingFrontsTest, RoofsDropOntoTheirBuildings) {
    EXPECT_GT(roofDropPixels(204), 0);
    EXPECT_GT(roofDropPixels(205), 0);
}

TEST(BuildingFrontsTest, NonRoofsHaveNoDrop) {
    EXPECT_EQ(roofDropPixels(0), 0);
    EXPECT_EQ(roofDropPixels(201), 0);
}

TEST(BuildingFrontsTest, NonRoofsHaveNoHorizontalShift) {
    EXPECT_EQ(roofShiftX(0), 0);
    EXPECT_EQ(roofShiftX(201), 0);
}
