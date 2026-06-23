#include <gtest/gtest.h>

#include "CityStamp.h"
#include "DungeonStamp.h"
#include "ForestPrefab.h"
#include "ForestStamp.h"

namespace {
EditorMap emptyMap() { return EditorMap(100, 100, 32, "world/tileset.png", 32); }
}  // namespace

TEST(ForestStampTest, ApplyStampsTreesObstaclesAndRect) {
    EditorMap map = emptyMap();
    applyForestPrefab(map, 10, 20);
    const ForestCell& oak = getForestPrefab().decoration.front();
    EXPECT_EQ(map.getDecoration()[20 + oak.dy][10 + oak.dx], 11);
    EXPECT_TRUE(map.isBlocked(10 + oak.dx + 3, 20 + oak.dy));
    ASSERT_EQ(map.getForests().size(), 1u);
    EXPECT_EQ(map.getForests()[0].x, 10);
    EXPECT_EQ(map.getForests()[0].y, 20);
    EXPECT_EQ(map.getForests()[0].width, 12);
    EXPECT_EQ(map.getForests()[0].height, 12);
}

TEST(ForestStampTest, RejectsWhenOutOfBounds) {
    EditorMap map = emptyMap();
    EXPECT_NE(forestStampError(map, 95, 10), "");
    EXPECT_NE(forestStampError(map, 10, 95), "");
    EXPECT_NE(forestStampError(map, -1, 10), "");
    EXPECT_EQ(forestStampError(map, 10, 10), "");
}

TEST(ForestStampTest, RejectsWhenOverlappingCityOrDungeonOrForest) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 10, "Pueblo");
    EXPECT_NE(forestStampError(map, 15, 15), "");
    applyDungeonPrefab(map, 60, 10);
    EXPECT_NE(forestStampError(map, 62, 12), "");
    applyForestPrefab(map, 80, 80);
    EXPECT_NE(forestStampError(map, 84, 84), "");
    EXPECT_EQ(forestStampError(map, 60, 60), "");
}

TEST(ForestStampTest, OriginForClickCentersTheBlock) {
    CellPos origin = forestOriginForClick(50, 40);
    EXPECT_EQ(origin.x, 50 - 6);
    EXPECT_EQ(origin.y, 40 - 6);
}

TEST(ForestStampTest, EraseForestAtRemovesEverything) {
    EditorMap map = emptyMap();
    applyForestPrefab(map, 10, 20);
    const ForestCell& oak = getForestPrefab().decoration.front();
    ASSERT_EQ(map.getDecoration()[20 + oak.dy][10 + oak.dx], 11);

    EXPECT_TRUE(eraseForestAt(map, 10 + 1, 20 + 1));

    EXPECT_EQ(map.getDecoration()[20 + oak.dy][10 + oak.dx], 0);
    EXPECT_FALSE(map.isBlocked(10 + oak.dx + 3, 20 + oak.dy));
    EXPECT_TRUE(map.getForests().empty());
}

TEST(ForestStampTest, EraseForestAtReturnsFalseWhenNoneThere) {
    EditorMap map = emptyMap();
    EXPECT_FALSE(eraseForestAt(map, 5, 5));
}
