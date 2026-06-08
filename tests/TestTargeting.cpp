#include <gtest/gtest.h>

#include "../client/src/core/Targeting.h"

TEST(TargetingTest, ScreenToCellNoOffset) {
    Cell cell = screenToCell(40, 70, 0, 0, 32);
    EXPECT_EQ(cell.col, 1);
    EXPECT_EQ(cell.row, 2);
}

TEST(TargetingTest, ScreenToCellWithCameraOffset) {
    Cell cell = screenToCell(10, 10, 64, 32, 32);
    EXPECT_EQ(cell.col, 2);
    EXPECT_EQ(cell.row, 1);
}

TEST(TargetingTest, PickTargetReturnsMonsterOnCell) {
    SnapshotDTO snap;
    snap.players.push_back(EntityDTO(1, EntityType::PLAYER, 5, 5, 100, 100, 0));
    snap.monsters.push_back(EntityDTO(200, EntityType::MONSTER, 5, 6, 10, 20, 0));
    auto target = pickTargetAt(5, 6, snap, 1, 1);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(*target, 200u);
}

TEST(TargetingTest, PickTargetExcludesSelf) {
    SnapshotDTO snap;
    snap.players.push_back(EntityDTO(1, EntityType::PLAYER, 5, 6, 100, 100, 0));
    auto target = pickTargetAt(5, 6, snap, 1, 1);
    EXPECT_FALSE(target.has_value());
}

TEST(TargetingTest, PickTargetEmptyCellReturnsNullopt) {
    SnapshotDTO snap;
    snap.players.push_back(EntityDTO(1, EntityType::PLAYER, 5, 5, 100, 100, 0));
    snap.monsters.push_back(EntityDTO(200, EntityType::MONSTER, 5, 6, 10, 20, 0));
    auto target = pickTargetAt(5, 4, snap, 1, 1);
    EXPECT_FALSE(target.has_value());
}

TEST(TargetingTest, PickTargetSkipsDeadEntity) {
    SnapshotDTO snap;
    snap.players.push_back(EntityDTO(1, EntityType::PLAYER, 5, 5, 100, 100, 0));
    snap.monsters.push_back(EntityDTO(200, EntityType::MONSTER, 5, 6, 0, 20, 0));
    auto target = pickTargetAt(5, 6, snap, 1, 1);
    EXPECT_FALSE(target.has_value());
}

TEST(TargetingTest, PickTargetInRangeSucceeds) {
    SnapshotDTO snap;
    snap.players.push_back(EntityDTO(1, EntityType::PLAYER, 5, 5, 100, 100, 0));
    snap.monsters.push_back(EntityDTO(200, EntityType::MONSTER, 6, 5, 10, 20, 0));
    auto target = pickTargetAt(6, 5, snap, 1, 1);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(*target, 200u);
}

TEST(TargetingTest, PickTargetOutOfRangeReturnsNullopt) {
    SnapshotDTO snap;
    snap.players.push_back(EntityDTO(1, EntityType::PLAYER, 5, 5, 100, 100, 0));
    snap.monsters.push_back(EntityDTO(200, EntityType::MONSTER, 9, 5, 10, 20, 0));
    auto target = pickTargetAt(9, 5, snap, 1, 1);
    EXPECT_FALSE(target.has_value());
}

TEST(TargetingTest, FindEntityByIdReturnsEntity) {
    SnapshotDTO snap;
    snap.monsters.push_back(EntityDTO(200, EntityType::MONSTER, 5, 6, 10, 20, 0));
    const EntityDTO* found = findEntityById(snap, 200);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->x, 5);
}

TEST(TargetingTest, FindEntityByIdMissingReturnsNull) {
    SnapshotDTO snap;
    EXPECT_EQ(findEntityById(snap, 999), nullptr);
}
