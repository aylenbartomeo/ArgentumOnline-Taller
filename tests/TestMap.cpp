#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "../server/src/Map.h"

// =======================================================================
// TESTS DE GEOMETRÍA Y LÍMITES
// =======================================================================

TEST(MapTest, Map_DimensionsAreCorrectlyReported) {
    Map mapa;
    mapa.setDimensions(80, 60);

    EXPECT_EQ(mapa.widthLimit(), 80);
    EXPECT_EQ(mapa.heightLimit(), 60);
}

TEST(MapTest, Map_InitialPositionIsInsideAndSafe) {
    Map mapa;
    mapa.setDimensions(100, 100);
    mapa.setSpawnPoint(15.5f, 22.3f);

    std::pair<float, float> spawn = mapa.getInitialPosition();

    EXPECT_FLOAT_EQ(spawn.first, 15.5f);
    EXPECT_FLOAT_EQ(spawn.second, 22.3f);
    EXPECT_FALSE(mapa.playerColision(spawn.first, spawn.second));
}

TEST(MapTest, Map_DetectsOutOfBoundsAsCollision) {
    Map mapa;
    mapa.setDimensions(50, 50);

    EXPECT_TRUE(mapa.playerColision(0.2f, 25.0f));
    EXPECT_TRUE(mapa.playerColision(25.0f, 0.2f));

    EXPECT_TRUE(mapa.playerColision(49.8f, 25.0f));
    EXPECT_TRUE(mapa.playerColision(25.0f, 49.8f));
}

// =======================================================================
// TESTS DE GUARDADO Y COLISIÓN DE OBSTÁCULOS
// =======================================================================

TEST(MapTest, Map_ObstacleIsSavedInElementsAndGrid) {
    Map mapa;
    mapa.setDimensions(30, 30);

    mapa.setObstacleInGrid(12, 14, true);

    const auto& elementos = mapa.getElements();
    ASSERT_FALSE(elementos.empty());
    EXPECT_EQ(elementos.back().type, MapElementType::OBSTACLE);
    EXPECT_EQ(elementos.back().area.x, 12);
    EXPECT_EQ(elementos.back().area.y, 14);

    EXPECT_TRUE(mapa.playerColision(12.5f, 14.5f));
}

TEST(MapTest, Map_RegenerateCollisionGridFromElements) {
    Map mapa;
    mapa.setDimensions(20, 20);

    mapa.setObstacleInGrid(5, 5, true);

    EXPECT_TRUE(mapa.playerColision(5.5f, 5.5f));
}

// =======================================================================
// TESTS DE HITBOX DE ATAQUE Y ZONAS
// =======================================================================

TEST(MapTest, Map_HasLineOfSight_Bresenham) {
    Map mapa;
    mapa.setDimensions(20, 20);

    EXPECT_TRUE(mapa.hasLineOfSight(Position{0, 0}, Position{10, 10}));
    EXPECT_TRUE(mapa.hasLineOfSight(Position{5, 5}, Position{15, 5}));

    mapa.setObstacleInGrid(5, 5, true);

    EXPECT_FALSE(mapa.hasLineOfSight(Position{0, 0}, Position{10, 10}));
    EXPECT_FALSE(mapa.hasLineOfSight(Position{2, 5}, Position{8, 5}));

    EXPECT_TRUE(mapa.hasLineOfSight(Position{0, 0}, Position{0, 10}));
    EXPECT_TRUE(mapa.hasLineOfSight(Position{2, 4}, Position{8, 4}));
}

TEST(MapTest, Map_CorrectlyIdentifiesCitizenArea) {
    Map mapa;
    mapa.setDimensions(100, 100);

    mapa.setCitizenArea(10, 10, 20, 20);

    EXPECT_TRUE(mapa.isCitizenArea(15.0f, 15.0f));
    EXPECT_FALSE(mapa.isCitizenArea(5.0f, 15.0f));
}

TEST(MapTest, Map_LoadsSpawnAndDimensionsFromJson) {
    // Arrange
    const std::string path = std::string("/tmp/") +
                             ::testing::UnitTest::GetInstance()->current_test_info()->name() +
                             ".json";
    std::ofstream out(path);
    out << R"({"tileSize":16,"tileset":"x.png","tilesetCols":12,"width":25,"height":18,)"
        << R"("spawn":{"x":7,"y":9},"tiles":[]})";
    out.close();
    Map mapa;

    // Act
    bool success = mapa.loadSpawnFromJson(path);
    std::pair<float, float> spawn = mapa.getInitialPosition();

    // Assert
    ASSERT_TRUE(success);
    EXPECT_EQ(mapa.widthLimit(), 25);
    EXPECT_EQ(mapa.heightLimit(), 18);
    EXPECT_FLOAT_EQ(spawn.first, 7.0f);
    EXPECT_FLOAT_EQ(spawn.second, 9.0f);

    std::filesystem::remove(path);
}

TEST(MapTest, Map_DungeonFromJsonBecomesBossZoneCenteredInArena) {
    // Arrange
    const std::string path = std::string("/tmp/") +
                             ::testing::UnitTest::GetInstance()->current_test_info()->name() +
                             ".json";
    std::ofstream out(path);
    out << R"({"width":100,"height":100,"spawn":{"x":0,"y":0},)"
        << R"("dungeons":[{"x":40,"y":50,"width":14,"height":14}]})";
    out.close();
    Map mapa;

    // Act
    bool success = mapa.loadSpawnFromJson(path);
    const auto& bossZones = mapa.getBossZones();

    // Assert
    ASSERT_TRUE(success);
    ASSERT_EQ(bossZones.size(), 1u);
    const BossZoneConfig& bz = bossZones[0];
    EXPECT_EQ(bz.x, 40);
    EXPECT_EQ(bz.y, 50);
    EXPECT_EQ(bz.width, 14);
    EXPECT_EQ(bz.height, 14);
    EXPECT_EQ(bz.spawnX, 47);
    EXPECT_EQ(bz.spawnY, 57);
    EXPECT_FLOAT_EQ(bz.respawnCooldownMs, 300000.0f);

    std::filesystem::remove(path);
}

TEST(MapTest, Map_LoadSpawnFromJsonFailsOnMissingFile) {
    Map mapa;
    EXPECT_FALSE(mapa.loadSpawnFromJson("/tmp/no_existe_este_mapa_12345.json"));
}

TEST(MapTest, Map_CanMoveToFarColumnInWideMap) {
    Map mapa;
    mapa.setDimensions(20, 15);

    EXPECT_TRUE(mapa.canMoveTo(Position{18, 3}));
}

// =======================================================================
// TESTS DE ITEMS EN EL SUELO
// =======================================================================

TEST(MapTest, Map_PlaceItem_and_PickUp) {
    Map mapa;
    mapa.setDimensions(20, 20);
    Position pos{5, 5};

    EXPECT_TRUE(mapa.placeItem(pos, 1, 10));
    EXPECT_TRUE(mapa.hasItemAt(pos));

    auto picked = mapa.pickUpItem(pos);
    ASSERT_TRUE(picked.has_value());
    EXPECT_EQ(picked->itemId, 1);
    EXPECT_EQ(picked->amount, 10);
    EXPECT_FALSE(mapa.hasItemAt(pos));
}

TEST(MapTest, Map_PlaceItemNearby_overflows_to_adjacent) {
    Map mapa;
    mapa.setDimensions(20, 20);
    Position pos{10, 10};

    auto pos1 = mapa.placeItemNearby(pos, 1, 1);
    ASSERT_TRUE(pos1.has_value());
    EXPECT_EQ(pos1->x, 10);
    EXPECT_EQ(pos1->y, 10);

    auto pos2 = mapa.placeItemNearby(pos, 2, 1);
    ASSERT_TRUE(pos2.has_value());
    EXPECT_NE(pos2.value(), pos);
    EXPECT_LE(pos2->distance_to(pos), 2);
}
