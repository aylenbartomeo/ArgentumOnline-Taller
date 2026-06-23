#include <algorithm>

#include <gtest/gtest.h>

#include "DungeonPrefab.h"

namespace {
bool hasObstacle(const DungeonPrefab& p, int dx, int dy) {
    return std::any_of(p.obstacles.begin(), p.obstacles.end(),
                       [dx, dy](const DungeonCell& c) { return c.dx == dx && c.dy == dy; });
}

int groundAt(const DungeonPrefab& p, int dx, int dy) {
    auto it = std::find_if(p.ground.rbegin(), p.ground.rend(),
                           [dx, dy](const DungeonCell& c) { return c.dx == dx && c.dy == dy; });
    return (it != p.ground.rend()) ? it->value : -1;
}

int decorationAt(const DungeonPrefab& p, int dx, int dy) {
    auto it = std::find_if(p.decoration.begin(), p.decoration.end(),
                           [dx, dy](const DungeonCell& c) { return c.dx == dx && c.dy == dy; });
    return (it != p.decoration.end()) ? it->value : -1;
}
}  // namespace

TEST(DungeonPrefabTest, HasFixedFootprintAndDungeonRect) {
    const DungeonPrefab& p = getDungeonPrefab();
    EXPECT_EQ(p.width, 16);
    EXPECT_EQ(p.height, 22);
    EXPECT_EQ(p.dungeonDx, 1);
    EXPECT_EQ(p.dungeonDy, 1);
    EXPECT_EQ(p.dungeonW, 14);
    EXPECT_EQ(p.dungeonH, 14);
}

TEST(DungeonPrefabTest, FloorInsideLavaRingAndCorridor) {
    const DungeonPrefab& p = getDungeonPrefab();
    EXPECT_EQ(groundAt(p, 1, 1), 4);
    EXPECT_EQ(groundAt(p, 0, 0), 45);
    EXPECT_EQ(groundAt(p, 15, 0), 45);
    EXPECT_EQ(groundAt(p, 7, 15), 4);
    EXPECT_EQ(groundAt(p, 7, 16), 4);
    EXPECT_EQ(groundAt(p, 6, 16), 45);
    EXPECT_EQ(groundAt(p, 9, 16), 45);
}

TEST(DungeonPrefabTest, LavaIsBlockedButFloorAndOpeningAreNot) {
    const DungeonPrefab& p = getDungeonPrefab();
    EXPECT_TRUE(hasObstacle(p, 0, 0));
    EXPECT_TRUE(hasObstacle(p, 6, 16));
    EXPECT_FALSE(hasObstacle(p, 1, 1));
    EXPECT_FALSE(hasObstacle(p, 7, 15));
    EXPECT_FALSE(hasObstacle(p, 7, 16));
}

TEST(DungeonPrefabTest, GraveWithHoleBlocksAFourCellColumnShiftedRight) {
    const DungeonPrefab& p = getDungeonPrefab();
    EXPECT_EQ(decorationAt(p, 2, 7), 102);
    EXPECT_FALSE(hasObstacle(p, 2, 7));
    EXPECT_TRUE(hasObstacle(p, 3, 7));
    EXPECT_TRUE(hasObstacle(p, 3, 6));
    EXPECT_TRUE(hasObstacle(p, 3, 5));
    EXPECT_TRUE(hasObstacle(p, 3, 4));
    EXPECT_FALSE(hasObstacle(p, 3, 3));
}

TEST(DungeonPrefabTest, SmallGraveBlocksOneCell) {
    const DungeonPrefab& p = getDungeonPrefab();
    EXPECT_EQ(decorationAt(p, 2, 12), 100);
    EXPECT_TRUE(hasObstacle(p, 2, 12));
    EXPECT_FALSE(hasObstacle(p, 2, 11));
}

TEST(DungeonPrefabTest, HasFourWalkableSkeletons) {
    const DungeonPrefab& p = getDungeonPrefab();
    int count = 0;
    for (const DungeonCell& c: p.decoration) {
        if (c.value == 99) {
            ++count;
            EXPECT_FALSE(hasObstacle(p, c.dx, c.dy));
        }
    }
    EXPECT_EQ(count, 4);
}

TEST(DungeonPrefabTest, GoldGuardedBehindBossAndAtEntrance) {
    const DungeonPrefab& p = getDungeonPrefab();
    ASSERT_EQ(p.gold.size(), 3u);
    int guarded = 0;
    int entrance = 0;
    for (const DungeonItem& g: p.gold) {
        EXPECT_EQ(g.itemId, 1);
        if (g.amount == 300) {
            ++guarded;
            EXPECT_EQ(g.dy, 1);
        } else if (g.amount == 50) {
            ++entrance;
            EXPECT_GE(g.dy, 16);
        }
    }
    EXPECT_EQ(guarded, 2);
    EXPECT_EQ(entrance, 1);
}
