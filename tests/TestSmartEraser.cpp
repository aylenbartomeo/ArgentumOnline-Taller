#include <gtest/gtest.h>

#include "EditorMap.h"
#include "SmartEraser.h"

namespace {
EditorMap emptyMap() { return EditorMap(30, 30, 32, "world/tileset.png", 32); }
}  // namespace

TEST(SmartEraserTest, EmptyCellClassifiesAsNone) {
    EditorMap map = emptyMap();
    EXPECT_EQ(topErasableAt(map, 5, 5).layer, EraseLayer::NONE);
}

TEST(SmartEraserTest, EmptyCellEraseIsNoOp) {
    EditorMap map = emptyMap();
    smartEraseAt(map, 5, 5);
    EXPECT_TRUE(map.getMonsters().empty());
    EXPECT_TRUE(map.getItems().empty());
}

TEST(SmartEraserTest, MonsterIsOnTop) {
    EditorMap map = emptyMap();
    map.addSafeZone("Pueblo", 4, 4, 6, 6);
    map.addCitizen("priest", 5, 5);
    map.setItem(5, 5, 0, 1);
    map.addMonster("goblin", 5, 5);
    EXPECT_EQ(topErasableAt(map, 5, 5).layer, EraseLayer::MONSTER);
}

TEST(SmartEraserTest, PeelsMonsterThenCitizenThenItemThenCity) {
    EditorMap map = emptyMap();
    map.addSafeZone("Pueblo", 4, 4, 6, 6);
    map.addMonster("goblin", 5, 5);
    map.addCitizen("priest", 5, 5);
    map.setItem(5, 5, 0, 1);

    smartEraseAt(map, 5, 5);
    EXPECT_TRUE(map.getMonsters().empty());
    EXPECT_FALSE(map.getCitizens().empty());

    smartEraseAt(map, 5, 5);
    EXPECT_TRUE(map.getCitizens().empty());
    EXPECT_NE(map.itemAt(5, 5), nullptr);

    smartEraseAt(map, 5, 5);
    EXPECT_EQ(map.itemAt(5, 5), nullptr);
    EXPECT_FALSE(map.getSafeZones().empty());

    smartEraseAt(map, 5, 5);
    EXPECT_TRUE(map.getSafeZones().empty());

    smartEraseAt(map, 5, 5);
    EXPECT_EQ(topErasableAt(map, 5, 5).layer, EraseLayer::NONE);
}

TEST(SmartEraserTest, DetectsAndErasesDungeonBlock) {
    EditorMap map = emptyMap();
    map.addDungeon(8, 8, 6, 6);
    EraseTarget t = topErasableAt(map, 9, 9);
    EXPECT_EQ(t.layer, EraseLayer::BLOCK);
    EXPECT_EQ(t.block, BlockKind::DUNGEON);
    smartEraseAt(map, 9, 9);
    EXPECT_TRUE(map.getDungeons().empty());
}

TEST(SmartEraserTest, DetectsAndErasesForestBlock) {
    EditorMap map = emptyMap();
    map.addForest(2, 2, 4, 4);
    EraseTarget t = topErasableAt(map, 3, 3);
    EXPECT_EQ(t.layer, EraseLayer::BLOCK);
    EXPECT_EQ(t.block, BlockKind::FOREST);
    smartEraseAt(map, 3, 3);
    EXPECT_TRUE(map.getForests().empty());
}

TEST(SmartEraserTest, DetectsAndErasesDesertBlock) {
    EditorMap map = emptyMap();
    map.addDesert(8, 8, 6, 6);
    EraseTarget t = topErasableAt(map, 9, 9);
    EXPECT_EQ(t.layer, EraseLayer::BLOCK);
    EXPECT_EQ(t.block, BlockKind::DESERT);
    smartEraseAt(map, 9, 9);
    EXPECT_TRUE(map.getDeserts().empty());
}

TEST(SmartEraserTest, DetectsAndErasesBeachBlock) {
    EditorMap map = emptyMap();
    map.addBeach(8, 8, 6, 6);
    EraseTarget t = topErasableAt(map, 9, 9);
    EXPECT_EQ(t.layer, EraseLayer::BLOCK);
    EXPECT_EQ(t.block, BlockKind::BEACH);
    smartEraseAt(map, 9, 9);
    EXPECT_TRUE(map.getBeaches().empty());
}

TEST(SmartEraserTest, OutOfBoundsIsSafe) {
    EditorMap map = emptyMap();
    EXPECT_EQ(topErasableAt(map, -1, -1).layer, EraseLayer::NONE);
    smartEraseAt(map, -1, -1);
    smartEraseAt(map, 999, 999);
    SUCCEED();
}
