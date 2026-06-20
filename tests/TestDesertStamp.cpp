#include <gtest/gtest.h>

#include "CityStamp.h"
#include "DesertPrefab.h"
#include "DesertStamp.h"
#include "DungeonStamp.h"
#include "ForestStamp.h"

namespace {
EditorMap emptyMap() { return EditorMap(100, 100, 32, "5108.png", 32); }
}  // namespace

TEST(DesertStampTest, ApplyPaintsSandAndRect) {
    EditorMap map = emptyMap();
    applyDesertPrefab(map, 10, 20);
    EXPECT_EQ(map.getGround()[20 + 8][10 + 8], 74);
    EXPECT_NE(map.getGround()[20][10], 74);
    ASSERT_EQ(map.getDeserts().size(), 1u);
    EXPECT_EQ(map.getDeserts()[0].x, 10);
    EXPECT_EQ(map.getDeserts()[0].y, 20);
    EXPECT_EQ(map.getDeserts()[0].width, 16);
    EXPECT_EQ(map.getDeserts()[0].height, 16);
}

TEST(DesertStampTest, RejectsWhenOutOfBounds) {
    EditorMap map = emptyMap();
    EXPECT_NE(desertStampError(map, 90, 10), "");
    EXPECT_NE(desertStampError(map, 10, 90), "");
    EXPECT_NE(desertStampError(map, -1, 10), "");
    EXPECT_EQ(desertStampError(map, 10, 10), "");
}

TEST(DesertStampTest, RejectsWhenOverlappingCityDungeonForestOrDesert) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 10, "Pueblo");
    EXPECT_NE(desertStampError(map, 15, 15), "");
    applyDungeonPrefab(map, 60, 10);
    EXPECT_NE(desertStampError(map, 62, 12), "");
    applyForestPrefab(map, 80, 5);
    EXPECT_NE(desertStampError(map, 82, 7), "");
    applyDesertPrefab(map, 40, 60);
    EXPECT_NE(desertStampError(map, 44, 64), "");
    EXPECT_EQ(desertStampError(map, 5, 80), "");
}

TEST(DesertStampTest, OriginForClickCentersTheBlock) {
    CellPos origin = desertOriginForClick(50, 40);
    EXPECT_EQ(origin.x, 50 - 8);
    EXPECT_EQ(origin.y, 40 - 8);
}

TEST(DesertStampTest, EraseDesertAtRepaintsGrassAndRemovesRect) {
    EditorMap map = emptyMap();
    applyDesertPrefab(map, 10, 20);
    ASSERT_EQ(map.getGround()[20 + 8][10 + 8], 74);

    EXPECT_TRUE(eraseDesertAt(map, 10 + 1, 20 + 1));

    EXPECT_EQ(map.getGround()[20 + 8][10 + 8], 108);
    EXPECT_TRUE(map.getDeserts().empty());
}

TEST(DesertStampTest, EraseDesertAtReturnsFalseWhenNoneThere) {
    EditorMap map = emptyMap();
    EXPECT_FALSE(eraseDesertAt(map, 5, 5));
}
