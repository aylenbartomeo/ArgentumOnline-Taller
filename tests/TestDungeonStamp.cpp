#include <gtest/gtest.h>

#include "CityStamp.h"
#include "DungeonStamp.h"

namespace {
EditorMap emptyMap() { return EditorMap(100, 100, 32, "5108.png", 32); }
}  // namespace

TEST(DungeonStampTest, ApplyStampsFloorLavaDecorationGoldAndRect) {
    EditorMap map = emptyMap();
    applyDungeonPrefab(map, 10, 20);

    EXPECT_EQ(map.getGround()[20 + 1][10 + 1], 4);
    EXPECT_EQ(map.getGround()[20 + 0][10 + 0], 45);
    EXPECT_TRUE(map.isBlocked(10 + 0, 20 + 0));
    EXPECT_FALSE(map.isBlocked(10 + 7, 20 + 15));

    EXPECT_EQ(map.getDecoration()[20 + 7][10 + 2], 102);
    EXPECT_EQ(map.getDecoration()[20 + 4][10 + 5], 99);
    EXPECT_TRUE(map.isBlocked(10 + 2, 20 + 4));
    EXPECT_FALSE(map.isBlocked(10 + 5, 20 + 4));

    const PlacedItem* gold = map.itemAt(10 + 7, 20 + 1);
    ASSERT_NE(gold, nullptr);
    EXPECT_EQ(gold->amount, 5000);

    ASSERT_EQ(map.getDungeons().size(), 1u);
    EXPECT_EQ(map.getDungeons()[0].x, 11);
    EXPECT_EQ(map.getDungeons()[0].y, 21);
    EXPECT_EQ(map.getDungeons()[0].width, 14);
    EXPECT_EQ(map.getDungeons()[0].height, 14);
}

TEST(DungeonStampTest, RejectsWhenOutOfBounds) {
    EditorMap map = emptyMap();
    EXPECT_NE(dungeonStampError(map, 95, 10), "");
    EXPECT_NE(dungeonStampError(map, 10, 90), "");
    EXPECT_NE(dungeonStampError(map, -1, 10), "");
    EXPECT_EQ(dungeonStampError(map, 10, 10), "");
}

TEST(DungeonStampTest, RejectsWhenOverlappingACity) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 10, "Pueblo");
    EXPECT_NE(dungeonStampError(map, 20, 20), "");
    EXPECT_EQ(dungeonStampError(map, 60, 60), "");
}

TEST(DungeonStampTest, RejectsWhenOverlappingAnotherDungeon) {
    EditorMap map = emptyMap();
    applyDungeonPrefab(map, 10, 10);
    EXPECT_NE(dungeonStampError(map, 15, 15), "");
    EXPECT_EQ(dungeonStampError(map, 60, 60), "");
}

TEST(DungeonStampTest, OriginForClickCentersTheArena) {
    CellPos origin = dungeonOriginForClick(50, 40);
    EXPECT_EQ(origin.x, 50 - 8);
    EXPECT_EQ(origin.y, 40 - 8);
}

TEST(DungeonStampTest, EraseDungeonAtRemovesEverything) {
    EditorMap map = emptyMap();
    applyDungeonPrefab(map, 10, 20);
    ASSERT_TRUE(map.isBlocked(10 + 0, 20 + 0));

    EXPECT_TRUE(eraseDungeonAt(map, 10 + 5, 20 + 5));

    EXPECT_EQ(map.getGround()[20 + 0][10 + 0], 108);
    EXPECT_EQ(map.getDecoration()[20 + 7][10 + 2], 0);
    EXPECT_FALSE(map.isBlocked(10 + 0, 20 + 0));
    EXPECT_EQ(map.itemAt(10 + 7, 20 + 1), nullptr);
    EXPECT_TRUE(map.getDungeons().empty());
}

TEST(DungeonStampTest, EraseDungeonAtReturnsFalseWhenNoneThere) {
    EditorMap map = emptyMap();
    EXPECT_FALSE(eraseDungeonAt(map, 5, 5));
}

TEST(DungeonStampTest, MonstersRejectedInsideTheDungeon) {
    EditorMap map = emptyMap();
    applyDungeonPrefab(map, 10, 20);
    EXPECT_NE(monsterPlacementError(map, 10 + 5, 20 + 5), "");
    EXPECT_EQ(monsterPlacementError(map, 80, 80), "");
}

TEST(DungeonStampTest, CitizensAlreadyRejectedInsideTheDungeon) {
    EditorMap map = emptyMap();
    applyDungeonPrefab(map, 10, 20);
    EXPECT_NE(citizenPlacementError(map, "priest", 10 + 5, 20 + 5), "");
}
