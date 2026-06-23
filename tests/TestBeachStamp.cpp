#include <gtest/gtest.h>

#include "BeachStamp.h"
#include "CityStamp.h"
#include "DungeonStamp.h"

namespace {
EditorMap emptyMap() { return EditorMap(100, 100, 32, "world/tileset.png", 32); }
}  // namespace

TEST(BeachStampTest, ApplyPaintsWaterFoamAndBlocksWater) {
    EditorMap map = emptyMap();
    applyBeachPrefab(map, 10, 20);
    EXPECT_EQ(map.getGround()[20 + 8][10 + 8], 109);
    EXPECT_TRUE(map.isBlocked(10 + 8, 20 + 8));
    EXPECT_EQ(map.getGround2()[20 + 2][10 + 2], 87);
    ASSERT_EQ(map.getBeaches().size(), 1u);
    EXPECT_EQ(map.getBeaches()[0].width, 22);
    EXPECT_EQ(map.getBeaches()[0].height, 18);
}

TEST(BeachStampTest, RejectsWhenOutOfBounds) {
    EditorMap map = emptyMap();
    EXPECT_NE(beachStampError(map, 85, 10), "");
    EXPECT_NE(beachStampError(map, 10, 85), "");
    EXPECT_NE(beachStampError(map, -1, 10), "");
    EXPECT_EQ(beachStampError(map, 10, 10), "");
}

TEST(BeachStampTest, RejectsWhenOverlappingOtherBlocks) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 10, "Pueblo");
    EXPECT_NE(beachStampError(map, 15, 15), "");
    applyDungeonPrefab(map, 70, 5);
    EXPECT_NE(beachStampError(map, 68, 7), "");
    applyBeachPrefab(map, 5, 60);
    EXPECT_NE(beachStampError(map, 8, 63), "");
    EXPECT_EQ(beachStampError(map, 40, 60), "");
}

TEST(BeachStampTest, OriginForClickCentersTheBlock) {
    CellPos o = beachOriginForClick(50, 40);
    EXPECT_EQ(o.x, 50 - 11);
    EXPECT_EQ(o.y, 40 - 9);
}

TEST(BeachStampTest, EraseBeachRevertsEverything) {
    EditorMap map = emptyMap();
    applyBeachPrefab(map, 10, 20);
    ASSERT_EQ(map.getGround()[20 + 8][10 + 8], 109);

    EXPECT_TRUE(eraseBeachAt(map, 10 + 3, 20 + 3));

    EXPECT_EQ(map.getGround()[20 + 8][10 + 8], 108);
    EXPECT_EQ(map.getGround2()[20 + 2][10 + 2], 0);
    EXPECT_FALSE(map.isBlocked(10 + 8, 20 + 8));
    EXPECT_TRUE(map.getBeaches().empty());
}

TEST(BeachStampTest, EraseBeachAtReturnsFalseWhenNoneThere) {
    EditorMap map = emptyMap();
    EXPECT_FALSE(eraseBeachAt(map, 5, 5));
}
