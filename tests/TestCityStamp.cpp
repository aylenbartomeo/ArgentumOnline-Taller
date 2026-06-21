#include <gtest/gtest.h>

#include "CityPrefab.h"
#include "CityStamp.h"
#include "DungeonStamp.h"
#include "ForestStamp.h"

namespace {
constexpr int WATER = 109;

EditorMap emptyMap() { return EditorMap(100, 100, 32, "5108.png", 32); }
}  // namespace

TEST(CityStampTest, ApplyStampsAllLayersWithOffset) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");

    EXPECT_EQ(map.getGround()[20 + 23][10 + 1], 219);
    EXPECT_EQ(map.getGround()[20 + 18][10 + 20], 166);
    EXPECT_EQ(map.getDecoration()[20 + 22][10 + 2], 201);
    EXPECT_EQ(map.getDecoration()[20 + 18][10 + 20], 202);
    EXPECT_EQ(map.getDecoration()[20 + 30][10 + 16], 203);
    EXPECT_EQ(map.getRoofs()[20 + 22][10 + 2], 204);
    EXPECT_EQ(map.getRoofs()[20 + 18][10 + 20], 205);
    EXPECT_EQ(map.getIndoor()[20 + 14][10 + 9], 1);
    EXPECT_TRUE(map.isBlocked(10 + 2, 20 + 22));
    EXPECT_FALSE(map.isBlocked(10 + 9, 20 + 22));

    ASSERT_EQ(map.getCitizens().size(), 3u);
    EXPECT_EQ(map.getCitizens()[0].type, "priest");
    EXPECT_EQ(map.getCitizens()[0].x, 10 + 9);
    EXPECT_EQ(map.getCitizens()[0].y, 20 + 14);

    ASSERT_EQ(map.getSafeZones().size(), 1u);
    EXPECT_EQ(map.getSafeZones()[0].name, "Pueblo");
    EXPECT_EQ(map.getSafeZones()[0].x, 11);
    EXPECT_EQ(map.getSafeZones()[0].y, 25);
    EXPECT_EQ(map.getSafeZones()[0].width, 36);
    EXPECT_EQ(map.getSafeZones()[0].height, 29);
}

TEST(CityStampTest, RejectsWhenOutOfBounds) {
    EditorMap map = emptyMap();
    EXPECT_NE(cityStampError(map, 90, 10), "");
    EXPECT_NE(cityStampError(map, 10, 90), "");
    EXPECT_NE(cityStampError(map, -1, 10), "");
    EXPECT_EQ(cityStampError(map, 10, 10), "");
}

TEST(CityStampTest, RejectsWhenOverWater) {
    EditorMap map = emptyMap();
    map.setGround(30, 30, WATER);
    EXPECT_NE(cityStampError(map, 10, 10), "");
    EXPECT_EQ(cityStampError(map, 50, 50), "");
}

TEST(CityStampTest, RejectsWhenOverlappingAnotherCity) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 10, "Pueblo");
    EXPECT_NE(cityStampError(map, 30, 20), "");
    EXPECT_EQ(cityStampError(map, 54, 10), "");
}

TEST(CityStampTest, RejectsWhenOverlappingADungeon) {
    EditorMap map = emptyMap();
    applyDungeonPrefab(map, 60, 60);
    EXPECT_NE(cityStampError(map, 40, 40), "");
    EXPECT_EQ(cityStampError(map, 5, 5), "");
}

TEST(CityStampTest, RejectsWhenOverlappingAForest) {
    EditorMap map = emptyMap();
    applyForestPrefab(map, 60, 60);
    EXPECT_NE(cityStampError(map, 40, 40), "");
    EXPECT_EQ(cityStampError(map, 5, 5), "");
}

TEST(CityStampTest, ClearCityRestoresGrassAndRemovesEverything) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    map.addCitizen("priest", 10 + 5, 20 + 25);

    clearCity(map, 10, 20);

    EXPECT_EQ(map.getGround()[20 + 23][10 + 1], 108);
    EXPECT_EQ(map.getDecoration()[20 + 22][10 + 2], 0);
    EXPECT_EQ(map.getRoofs()[20 + 22][10 + 2], 0);
    EXPECT_EQ(map.getIndoor()[20 + 14][10 + 9], 0);
    EXPECT_FALSE(map.isBlocked(10 + 2, 20 + 22));
    EXPECT_TRUE(map.getCitizens().empty());
    EXPECT_TRUE(map.getSafeZones().empty());

    EXPECT_EQ(cityStampError(map, 10, 20), "");
}

TEST(CityStampTest, EraseCityAtFullyRemovesTheCityNotJustTheGround) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");

    ASSERT_EQ(map.getDecoration()[20 + 22][10 + 2], 201);
    ASSERT_TRUE(map.isBlocked(10 + 2, 20 + 22));

    EXPECT_TRUE(eraseCityAt(map, 20, 35));

    EXPECT_EQ(map.getDecoration()[20 + 22][10 + 2], 0);
    EXPECT_EQ(map.getDecoration()[20 + 18][10 + 20], 0);
    EXPECT_EQ(map.getDecoration()[20 + 30][10 + 16], 0);
    EXPECT_EQ(map.getRoofs()[20 + 22][10 + 2], 0);
    EXPECT_FALSE(map.isBlocked(10 + 2, 20 + 22));
    EXPECT_EQ(map.getGround()[20 + 23][10 + 1], 108);
    EXPECT_TRUE(map.getSafeZones().empty());
}

TEST(CityStampTest, EraseCityAtReturnsFalseWhenNoCityThere) {
    EditorMap map = emptyMap();
    EXPECT_FALSE(eraseCityAt(map, 5, 5));
}

TEST(CityStampTest, CityOriginForClickCentersTheCityOnTheClick) {
    CellPos origin = cityOriginForClick(50, 40);
    EXPECT_EQ(origin.x, 50 - 22);
    EXPECT_EQ(origin.y, 40 - 17);
}

TEST(CitizenPlacementTest, PriestGoesInsideTheChurch) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_EQ(citizenPlacementError(map, "priest", 10 + 9, 20 + 14), "");
}

TEST(CitizenPlacementTest, PriestRejectedOutsideTheChurch) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_NE(citizenPlacementError(map, "priest", 10 + 30, 20 + 10), "");
}

TEST(CitizenPlacementTest, PriestRejectedOnAWall) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_NE(citizenPlacementError(map, "priest", 10 + 2, 20 + 22), "");
}

TEST(CitizenPlacementTest, BankerGoesInsideTheBank) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_EQ(citizenPlacementError(map, "banker", 10 + 30, 20 + 13), "");
}

TEST(CitizenPlacementTest, BankerRejectedInsideTheChurch) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_NE(citizenPlacementError(map, "banker", 10 + 9, 20 + 14), "");
}

TEST(CitizenPlacementTest, MerchantGoesAroundTheStall) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_EQ(citizenPlacementError(map, "merchant", 10 + 20, 20 + 32), "");
}

TEST(CitizenPlacementTest, MerchantRejectedInsideTheChurch) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_NE(citizenPlacementError(map, "merchant", 10 + 9, 20 + 14), "");
}

TEST(CitizenPlacementTest, AnyCitizenRejectedOutsideAnyCity) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_NE(citizenPlacementError(map, "priest", 5, 5), "");
}

TEST(MonsterPlacementTest, RejectedInsideACityBuilding) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_NE(monsterPlacementError(map, "orc", 10 + 9, 20 + 14), "");
}

TEST(MonsterPlacementTest, RejectedAnywhereInTheCityNotJustBuildings) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_NE(monsterPlacementError(map, "orc", 10 + 2, 20 + 26), "");
}

TEST(MonsterPlacementTest, AllowedOutsideAnyCity) {
    EditorMap map = emptyMap();
    applyCityPrefab(map, 10, 20, "Pueblo");
    EXPECT_EQ(monsterPlacementError(map, "orc", 5, 5), "");
}

TEST(MonsterPlacementTest, SpiderAndGoblinOnlyInForest) {
    EditorMap map = emptyMap();
    map.addForest(40, 40, 12, 12);
    EXPECT_EQ(monsterPlacementError(map, "spider", 45, 45), "");
    EXPECT_EQ(monsterPlacementError(map, "goblin", 45, 45), "");
    EXPECT_NE(monsterPlacementError(map, "spider", 5, 5), "");
    EXPECT_NE(monsterPlacementError(map, "goblin", 5, 5), "");
}

TEST(MonsterPlacementTest, SkeletonAndGolemOnlyInDesert) {
    EditorMap map = emptyMap();
    map.addDesert(60, 60, 12, 12);
    EXPECT_EQ(monsterPlacementError(map, "skeleton", 65, 65), "");
    EXPECT_EQ(monsterPlacementError(map, "golem", 65, 65), "");
    EXPECT_NE(monsterPlacementError(map, "skeleton", 5, 5), "");
    EXPECT_NE(monsterPlacementError(map, "golem", 5, 5), "");
}

TEST(MonsterPlacementTest, ForestMonstersRejectedInDesertAndViceversa) {
    EditorMap map = emptyMap();
    map.addForest(40, 40, 12, 12);
    map.addDesert(60, 60, 12, 12);
    EXPECT_NE(monsterPlacementError(map, "spider", 65, 65), "");
    EXPECT_NE(monsterPlacementError(map, "skeleton", 45, 45), "");
}

TEST(MonsterPlacementTest, UnrestrictedMonstersGoAnywhereOutsideCity) {
    EditorMap map = emptyMap();
    EXPECT_EQ(monsterPlacementError(map, "orc", 5, 5), "");
    EXPECT_EQ(monsterPlacementError(map, "zombie", 5, 5), "");
}

TEST(MonsterPlacementTest, RejectedInsideABeach) {
    EditorMap map = emptyMap();
    map.addBeach(30, 30, 22, 18);
    EXPECT_NE(monsterPlacementError(map, "orc", 35, 35), "");
}

TEST(ItemPlacementTest, RejectedInsideABeach) {
    EditorMap map = emptyMap();
    map.addBeach(30, 30, 22, 18);
    EXPECT_NE(itemPlacementError(map, 35, 35), "");
}

TEST(ItemPlacementTest, AllowedOutsideABeach) {
    EditorMap map = emptyMap();
    map.addBeach(30, 30, 22, 18);
    EXPECT_EQ(itemPlacementError(map, 5, 5), "");
}
