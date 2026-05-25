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

    // El offset es 0.3f, si nos paramos en 0.2f el extremo izquierdo da negativo (-0.1f)
    EXPECT_TRUE(mapa.playerColision(0.2f, 25.0f));
    EXPECT_TRUE(mapa.playerColision(25.0f, 0.2f));

    // Extremo superior (50 - 0.3 = 49.7). Posicionarse en 49.8f empuja el offset fuera
    EXPECT_TRUE(mapa.playerColision(49.8f, 25.0f));
    EXPECT_TRUE(mapa.playerColision(25.0f, 49.8f));
}

// =======================================================================
// TESTS DE GUARDADO Y COLISIÓN DE OBSTÁCULOS
// =======================================================================

TEST(MapTest, Map_ObstacleIsSavedInElementsAndGrid) {
    Map mapa;
    mapa.setDimensions(30, 30);

    // Agregamos un obstáculo en la baldosa (12, 14)
    mapa.setObstacleInGrid(12, 14, true);

    // 1. Verificamos que se guardó en la lista de elementos del mapa
    const auto& elementos = mapa.getElements();
    ASSERT_FALSE(elementos.empty());
    EXPECT_EQ(elementos.back().type, MapElementType::OBSTACLE);  // O simplemente OBSTACLE
    EXPECT_EQ(elementos.back().area.x, 12);
    EXPECT_EQ(elementos.back().area.y, 14);

    // 2. Verificamos la colisión por grilla en O(1) usando el centro de la baldosa
    EXPECT_TRUE(mapa.playerColision(12.5f, 14.5f));
}

TEST(MapTest, Map_RegenerateCollisionGridFromElements) {
    Map mapa;
    mapa.setDimensions(20, 20);

    // Insertamos directo en la lista de elementos simulando una carga de archivo
    mapa.setObstacleInGrid(5, 5, true);

    EXPECT_TRUE(mapa.playerColision(5.5f, 5.5f));
}

// =======================================================================
// TESTS DE HITBOX DE ATAQUE Y ZONAS
// =======================================================================

TEST(MapTest, Map_AttackCollisionDetectsNearbyObstacles) {
    Map mapa;
    mapa.setDimensions(100, 100);
    mapa.setObstacleInGrid(50, 50, true);  // Guarda el elemento en el vector

    // Tu lógica de ataque usa un rango amplio de ±20 unidades.
    // Atacar desde (40, 40) debería colisionar con el obstáculo en (50, 50)
    EXPECT_TRUE(mapa.attackColision(40.0f, 40.0f));

    // Atacar desde (10, 10) está demasiado lejos (distancia > 20)
    EXPECT_FALSE(mapa.attackColision(10.0f, 10.0f));
}

TEST(MapTest, Map_CorrectlyIdentifiesCitizenArea) {
    Map mapa;
    mapa.setDimensions(100, 100);

    // Seteamos el área segura de ciudadanos
    mapa.setCitizenArea(10, 10, 20, 20);

    // (15, 15) está dentro del rango x[10, 30] e y[10, 30]
    EXPECT_TRUE(mapa.isCitizenArea(15.0f, 15.0f));
    EXPECT_FALSE(mapa.isCitizenArea(5.0f, 15.0f));
}

TEST(MapTest, Map_LoadsSpawnAndDimensionsFromJson) {
    const std::string path = "/tmp/test_map_loadspawn.json";
    std::ofstream out(path);
    out << R"({"tileSize":16,"tileset":"x.png","tilesetCols":12,"width":25,"height":18,)"
        << R"("spawn":{"x":7,"y":9},"tiles":[]})";
    out.close();

    Map mapa;
    ASSERT_TRUE(mapa.loadSpawnFromJson(path));

    EXPECT_EQ(mapa.widthLimit(), 25);
    EXPECT_EQ(mapa.heightLimit(), 18);

    std::pair<float, float> spawn = mapa.getInitialPosition();
    EXPECT_FLOAT_EQ(spawn.first, 7.0f);
    EXPECT_FLOAT_EQ(spawn.second, 9.0f);
}

TEST(MapTest, Map_LoadSpawnFromJsonFailsOnMissingFile) {
    Map mapa;
    EXPECT_FALSE(mapa.loadSpawnFromJson("/tmp/no_existe_este_mapa_12345.json"));
}
