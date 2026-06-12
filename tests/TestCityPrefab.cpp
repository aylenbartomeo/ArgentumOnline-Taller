#include <algorithm>

#include <gtest/gtest.h>

#include "CityPrefab.h"
#include "OverlayRegistry.h"
#include "TerrainRegistry.h"

TEST(CityPrefabTest, IsTenByTen) {
    CityPrefab city = makeCityPrefab();
    EXPECT_EQ(city.width, 10);
    EXPECT_EQ(city.height, 10);
    EXPECT_EQ(city.terrain.size(), 100u);
}

TEST(CityPrefabTest, HasThreeNpcsWithCorrectTypes) {
    CityPrefab city = makeCityPrefab();
    ASSERT_EQ(city.npcs.size(), 3u);
    auto hasType = [&city](const std::string& t) {
        return std::any_of(city.npcs.begin(), city.npcs.end(),
                           [&t](const CityNpc& n) { return n.type == t; });
    };
    EXPECT_TRUE(hasType("priest"));
    EXPECT_TRUE(hasType("banker"));
    EXPECT_TRUE(hasType("merchant"));
}

TEST(CityPrefabTest, WallsAreSolidAndDoorIsNot) {
    CityPrefab city = makeCityPrefab();
    auto isWallAt = [&city](int dx, int dy) {
        return std::any_of(city.overlays.begin(), city.overlays.end(),
                           [dx, dy](const CityOverlayCell& o) {
                               return o.dx == dx && o.dy == dy && o.tile == OverlayTile::WALL;
                           });
    };
    EXPECT_TRUE(isWallAt(3, 0));
    EXPECT_FALSE(isWallAt(4, 2));  // puerta de la iglesia
}

TEST(CityPrefabTest, InteriorsUseFloorTerrain) {
    CityPrefab city = makeCityPrefab();
    auto codeAt = [&city](int dx, int dy) {
        for (const auto& c : city.terrain)
            if (c.dx == dx && c.dy == dy) return c.code;
        return -1;
    };
    EXPECT_EQ(codeAt(1, 7), TerrainCode::WOOD);
    EXPECT_EQ(codeAt(0, 0), TerrainCode::STONE);
}
