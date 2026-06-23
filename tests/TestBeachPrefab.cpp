#include <algorithm>

#include <gtest/gtest.h>

#include "BeachPrefab.h"

namespace {
int at(const std::vector<BeachCell>& cells, int dx, int dy) {
    auto it = std::find_if(cells.begin(), cells.end(),
                           [dx, dy](const BeachCell& c) { return c.dx == dx && c.dy == dy; });
    if (it != cells.end()) {
        return it->value;
    }
    return -1;
}
}  // namespace

TEST(BeachPrefabTest, FootprintIs22x18) {
    const BeachPrefab& p = getBeachPrefab();
    EXPECT_EQ(p.width, 22);
    EXPECT_EQ(p.height, 18);
}

TEST(BeachPrefabTest, WaterInTheMiddle) {
    const BeachPrefab& p = getBeachPrefab();
    EXPECT_EQ(at(p.ground, 8, 8), 109);
    EXPECT_EQ(at(p.ground, 2, 2), 109);
    EXPECT_EQ(at(p.ground, 19, 15), 109);
}

TEST(BeachPrefabTest, FoamRingCornersAreCorrect) {
    const BeachPrefab& p = getBeachPrefab();
    EXPECT_EQ(at(p.ground2, 2, 2), 87);
    EXPECT_EQ(at(p.ground2, 18, 2), 92);
    EXPECT_EQ(at(p.ground2, 2, 14), 86);
    EXPECT_EQ(at(p.ground2, 18, 14), 93);
}

TEST(BeachPrefabTest, HasPalmsAndObstacles) {
    const BeachPrefab& p = getBeachPrefab();
    bool hasPalm = std::any_of(p.decoration.begin(), p.decoration.end(),
                               [](const BeachCell& c) { return c.value == 42; });
    EXPECT_TRUE(hasPalm);
    EXPECT_EQ(p.obstacles.size(), 200u);
}

TEST(BeachPrefabTest, PalmCollisionIsShiftedRightOntoTrunk) {
    const BeachPrefab& p = getBeachPrefab();
    auto hasObstacle = [&](int dx, int dy) {
        return std::any_of(p.obstacles.begin(), p.obstacles.end(),
                           [&](const BeachCell& c) { return c.dx == dx && c.dy == dy; });
    };
    EXPECT_TRUE(hasObstacle(2, 1));
    EXPECT_FALSE(hasObstacle(0, 1));
}
