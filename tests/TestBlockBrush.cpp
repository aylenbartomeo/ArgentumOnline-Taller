#include <gtest/gtest.h>

#include "BlockBrush.h"
#include "CityStamp.h"
#include "DungeonStamp.h"
#include "ForestStamp.h"

namespace {
EditorMap emptyMap() { return EditorMap(100, 100, 32, "5108.png", 32); }
}  // namespace

TEST(BlockBrushTest, ApplyCityStampsCenteredOnClick) {
    EditorMap map = emptyMap();
    applyBlock(map, TerrainBlock::CITY, 50, 50);
    ASSERT_EQ(map.getSafeZones().size(), 1u);
}

TEST(BlockBrushTest, ApplyCityMatchesDirectStampAtCenteredOrigin) {
    EditorMap viaBrush = emptyMap();
    applyBlock(viaBrush, TerrainBlock::CITY, 50, 50);
    EditorMap viaStamp = emptyMap();
    CellPos o = cityOriginForClick(50, 50);
    applyCityPrefab(viaStamp, o.x, o.y, "Ciudad 1");
    ASSERT_EQ(viaBrush.getSafeZones().size(), 1u);
    ASSERT_EQ(viaStamp.getSafeZones().size(), 1u);
    EXPECT_EQ(viaBrush.getSafeZones()[0].x, viaStamp.getSafeZones()[0].x);
    EXPECT_EQ(viaBrush.getSafeZones()[0].y, viaStamp.getSafeZones()[0].y);
}

TEST(BlockBrushTest, ApplyForestStampsCenteredOnClick) {
    EditorMap map = emptyMap();
    applyBlock(map, TerrainBlock::FOREST, 50, 50);
    CellPos o = forestOriginForClick(50, 50);
    ASSERT_EQ(map.getForests().size(), 1u);
    EXPECT_EQ(map.getForests()[0].x, o.x);
    EXPECT_EQ(map.getForests()[0].y, o.y);
}

TEST(BlockBrushTest, ApplyDungeonStampsCenteredOnClick) {
    EditorMap map = emptyMap();
    applyBlock(map, TerrainBlock::DUNGEON, 50, 50);
    ASSERT_EQ(map.getDungeons().size(), 1u);
}

TEST(BlockBrushTest, ApplyNoneDoesNothing) {
    EditorMap map = emptyMap();
    applyBlock(map, TerrainBlock::NONE, 50, 50);
    EXPECT_TRUE(map.getSafeZones().empty());
    EXPECT_TRUE(map.getForests().empty());
    EXPECT_TRUE(map.getDungeons().empty());
}

TEST(BlockBrushTest, StampErrorEmptyForValidCityPlacement) {
    EditorMap map = emptyMap();
    EXPECT_EQ(blockStampError(map, TerrainBlock::CITY, 50, 50), "");
}

TEST(BlockBrushTest, StampErrorRejectsOutOfBoundsForest) {
    EditorMap map = emptyMap();
    EXPECT_NE(blockStampError(map, TerrainBlock::FOREST, 2, 2), "");
}

TEST(BlockBrushTest, StampErrorReportsBeachAndDesertNotReady) {
    EditorMap map = emptyMap();
    EXPECT_NE(blockStampError(map, TerrainBlock::BEACH, 50, 50), "");
    EXPECT_NE(blockStampError(map, TerrainBlock::DESERT, 50, 50), "");
}
