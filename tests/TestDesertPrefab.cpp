#include <gtest/gtest.h>

#include "DesertPrefab.h"

namespace {
int valueAt(const DesertPrefab& p, int dx, int dy) {
    for (const DesertCell& c: p.ground) {
        if (c.dx == dx && c.dy == dy) {
            return c.value;
        }
    }
    return -1;
}
}  // namespace

TEST(DesertPrefabTest, Is16x16WithFullGround) {
    const DesertPrefab& p = getDesertPrefab();
    EXPECT_EQ(p.width, 16);
    EXPECT_EQ(p.height, 16);
    EXPECT_EQ(p.ground.size(), 256u);
}

TEST(DesertPrefabTest, InteriorIsSand) {
    const DesertPrefab& p = getDesertPrefab();
    EXPECT_EQ(valueAt(p, 8, 8), 74);
    EXPECT_EQ(valueAt(p, 1, 1), 74);
    EXPECT_EQ(valueAt(p, 14, 14), 74);
}

TEST(DesertPrefabTest, AllCellsAreSand) {
    const DesertPrefab& p = getDesertPrefab();
    EXPECT_EQ(valueAt(p, 0, 0), 74);
    EXPECT_EQ(valueAt(p, 15, 0), 74);
    EXPECT_EQ(valueAt(p, 0, 15), 74);
    EXPECT_EQ(valueAt(p, 15, 15), 74);
    EXPECT_EQ(valueAt(p, 8, 0), 74);
    EXPECT_EQ(valueAt(p, 0, 8), 74);
}

TEST(DesertPrefabTest, HasCactiButNoSign) {
    const DesertPrefab& p = getDesertPrefab();
    EXPECT_FALSE(p.decoration.empty());
    bool hasCactus = false;
    bool hasSign = false;
    for (const DesertCell& c: p.decoration) {
        if (c.value == 137) {
            hasCactus = true;
        }
        if (c.value == 162) {
            hasSign = true;
        }
    }
    EXPECT_TRUE(hasCactus);
    EXPECT_FALSE(hasSign);
}

TEST(DesertPrefabTest, CactiAreObstacles) {
    const DesertPrefab& p = getDesertPrefab();
    EXPECT_FALSE(p.obstacles.empty());
    EXPECT_EQ(p.obstacles.size(), 8u);
}
