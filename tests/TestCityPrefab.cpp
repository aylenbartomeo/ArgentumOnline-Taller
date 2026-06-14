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
    EXPECT_EQ(banker->dy, 13);

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
    EXPECT_EQ(prefab.indoor.size(), static_cast<size_t>(15 * 18 + 15 * 11));
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

TEST(CityPrefabTest, GroundHasDirtBaseStoneFloorsAndPath) {
    const CityPrefab& prefab = getCityPrefab();
    auto groundAt = [&prefab](int dx, int dy) {
        int v = 0;
        for (const CityCell& c: prefab.ground) {
            if (c.dx == dx && c.dy == dy) {
                v = c.value;
            }
        }
        return v;
    };
    EXPECT_EQ(groundAt(20, 18), 17);
    EXPECT_EQ(groundAt(9, 22), 17);
    EXPECT_EQ(groundAt(20, 23), 17);
    EXPECT_EQ(groundAt(18, 6), 106);
}

TEST(CityPrefabTest, HasBuildingZonesForChurchBankAndStore) {
    const CityPrefab& prefab = getCityPrefab();
    ASSERT_EQ(prefab.buildings.size(), 3u);

    auto zoneNamed = [&prefab](const std::string& name) -> const CityZone* {
        auto it = std::find_if(prefab.buildings.begin(), prefab.buildings.end(),
                               [&name](const CityZone& z) { return z.name == name; });
        return (it != prefab.buildings.end()) ? &(*it) : nullptr;
    };

    const CityZone* church = zoneNamed("church");
    ASSERT_NE(church, nullptr);
    EXPECT_EQ(church->dx, 2);
    EXPECT_EQ(church->dy, 5);
    EXPECT_EQ(church->width, 15);
    EXPECT_EQ(church->height, 18);

    const CityZone* bank = zoneNamed("bank");
    ASSERT_NE(bank, nullptr);
    EXPECT_EQ(bank->dx, 20);
    EXPECT_EQ(bank->dy, 8);
    EXPECT_EQ(bank->width, 15);
    EXPECT_EQ(bank->height, 11);

    const CityZone* store = zoneNamed("store");
    ASSERT_NE(store, nullptr);
    EXPECT_EQ(store->dx, 14);
    EXPECT_EQ(store->dy, 27);
    EXPECT_EQ(store->width, 13);
    EXPECT_EQ(store->height, 6);
}

TEST(CityPrefabTest, ChurchBackWallBandIsBlocked) {
    const CityPrefab& prefab = getCityPrefab();
    for (int y = 5; y <= 10; ++y) {
        EXPECT_TRUE(hasObstacle(prefab, 9, y)) << "iglesia fila " << y;
    }
    EXPECT_FALSE(hasObstacle(prefab, 9, 11));
}

TEST(CityPrefabTest, BankBackWallBandIsBlocked) {
    const CityPrefab& prefab = getCityPrefab();
    for (int y = 8; y <= 11; ++y) {
        EXPECT_TRUE(hasObstacle(prefab, 30, y)) << "banco fila " << y;
    }
    EXPECT_FALSE(hasObstacle(prefab, 30, 12));
}
