#include <algorithm>

#include <gtest/gtest.h>

#include "CityPrefab.h"

namespace {
bool hasObstacle(const CityPrefab& prefab, int dx, int dy) {
    return std::any_of(prefab.obstacles.begin(), prefab.obstacles.end(),
                       [dx, dy](const CityCell& c) { return c.dx == dx && c.dy == dy; });
}

const CityNpc* npcOfType(const CityPrefab& prefab, const std::string& type) {
    auto it = std::find_if(prefab.npcs.begin(), prefab.npcs.end(),
                           [&type](const CityNpc& npc) { return npc.type == type; });
    return (it != prefab.npcs.end()) ? &(*it) : nullptr;
}
}  // namespace

TEST(CityPrefabTest, HasExpectedFootprint) {
    const CityPrefab& prefab = getCityPrefab();
    EXPECT_EQ(prefab.width, 44);
    EXPECT_EQ(prefab.height, 34);
}

TEST(CityPrefabTest, HasThreeNpcsAtTheirBuildings) {
    const CityPrefab& prefab = getCityPrefab();
    ASSERT_EQ(prefab.npcs.size(), 3u);

    const CityNpc* priest = npcOfType(prefab, "priest");
    ASSERT_NE(priest, nullptr);
    EXPECT_EQ(priest->dx, 9);
    EXPECT_EQ(priest->dy, 14);

    const CityNpc* banker = npcOfType(prefab, "banker");
    ASSERT_NE(banker, nullptr);
    EXPECT_EQ(banker->dx, 30);
    EXPECT_EQ(banker->dy, 10);

    const CityNpc* merchant = npcOfType(prefab, "merchant");
    ASSERT_NE(merchant, nullptr);
    EXPECT_EQ(merchant->dx, 20);
    EXPECT_EQ(merchant->dy, 32);
}

TEST(CityPrefabTest, HasRoofAnchorsForChurchAndBank) {
    const CityPrefab& prefab = getCityPrefab();
    ASSERT_EQ(prefab.roofs.size(), 2u);
    EXPECT_EQ(prefab.roofs[0].dx, 2);
    EXPECT_EQ(prefab.roofs[0].dy, 22);
    EXPECT_EQ(prefab.roofs[0].value, 204);
    EXPECT_EQ(prefab.roofs[1].dx, 20);
    EXPECT_EQ(prefab.roofs[1].dy, 18);
    EXPECT_EQ(prefab.roofs[1].value, 205);
}

TEST(CityPrefabTest, IndoorCoversBothBuildingFootprints) {
    const CityPrefab& prefab = getCityPrefab();
    EXPECT_EQ(prefab.indoor.size(), static_cast<size_t>(15 * 18 + 20 * 11));
}

TEST(CityPrefabTest, ChurchDoorIsOpenAndWallsAreBlocked) {
    const CityPrefab& prefab = getCityPrefab();
    EXPECT_FALSE(hasObstacle(prefab, 8, 22));
    EXPECT_FALSE(hasObstacle(prefab, 9, 22));
    EXPECT_FALSE(hasObstacle(prefab, 10, 22));
    EXPECT_TRUE(hasObstacle(prefab, 2, 22));
    EXPECT_TRUE(hasObstacle(prefab, 16, 22));
    EXPECT_TRUE(hasObstacle(prefab, 2, 5));
}

TEST(CityPrefabTest, GroundHasPlazaAndBankFloor) {
    const CityPrefab& prefab = getCityPrefab();
    bool plaza = std::any_of(prefab.ground.begin(), prefab.ground.end(), [](const CityCell& c) {
        return c.dx == 1 && c.dy == 23 && c.value == 17;
    });
    bool bankFloor = std::any_of(prefab.ground.begin(), prefab.ground.end(), [](const CityCell& c) {
        return c.dx == 20 && c.dy == 18 && c.value == 106;
    });
    EXPECT_TRUE(plaza);
    EXPECT_TRUE(bankFloor);
}
