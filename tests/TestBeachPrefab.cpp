#include <gtest/gtest.h>

#include "BeachPrefab.h"

namespace {
int at(const std::vector<BeachCell>& cells, int dx, int dy) {
    for (const BeachCell& c: cells) {
        if (c.dx == dx && c.dy == dy) {
            return c.value;
        }
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
    bool hasPalm = false;
    for (const BeachCell& c: p.decoration) {
        if (c.value == 42) {
            hasPalm = true;
        }
    }
    EXPECT_TRUE(hasPalm);
    EXPECT_EQ(p.obstacles.size(), 200u);
}

TEST(BeachPrefabTest, PalmCollisionIsShiftedRightOntoTrunk) {
    const BeachPrefab& p = getBeachPrefab();
    auto hasObstacle = [&](int dx, int dy) {
        for (const BeachCell& c: p.obstacles) {
            if (c.dx == dx && c.dy == dy) {
                return true;
            }
        }
        return false;
    };
    EXPECT_TRUE(hasObstacle(2, 1));
    EXPECT_FALSE(hasObstacle(0, 1));
}
