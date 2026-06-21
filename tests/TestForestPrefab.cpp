#include <algorithm>

#include <gtest/gtest.h>

#include "ForestPrefab.h"

namespace {
bool blocked(const ForestPrefab& p, int dx, int dy) {
    return std::any_of(p.obstacles.begin(), p.obstacles.end(),
                       [dx, dy](const ForestCell& c) { return c.dx == dx && c.dy == dy; });
}
}  // namespace

TEST(ForestPrefabTest, IsTwelveByTwelve) {
    const ForestPrefab& p = getForestPrefab();
    EXPECT_EQ(p.width, 12);
    EXPECT_EQ(p.height, 12);
}

TEST(ForestPrefabTest, HasSevenOaks) {
    const ForestPrefab& p = getForestPrefab();
    int oaks = std::count_if(p.decoration.begin(), p.decoration.end(),
                             [](const ForestCell& c) { return c.value == 11; });
    EXPECT_EQ(oaks, 7);
}

TEST(ForestPrefabTest, EachOakBlocksItsTwoTrunkCells) {
    const ForestPrefab& p = getForestPrefab();
    for (const ForestCell& oak: p.decoration) {
        if (oak.value != 11) {
            continue;
        }
        EXPECT_TRUE(blocked(p, oak.dx + 3, oak.dy)) << "roble " << oak.dx << "," << oak.dy;
        EXPECT_TRUE(blocked(p, oak.dx + 4, oak.dy)) << "roble " << oak.dx << "," << oak.dy;
    }
}
