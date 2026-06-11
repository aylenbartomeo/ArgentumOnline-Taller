#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "CityStamp.h"
#include "OverlayRegistry.h"
#include "TerrainRegistry.h"

TEST(CityStampTest, StampWritesNpcsSafeZoneAndObstacles) {
    EditorMap map(60, 60, 32, "5108.png", 32);
    applyCityPrefab(map, 25, 25, "Ullathorpe");

    EXPECT_EQ(map.getCitizens().size(), 3u);
    EXPECT_EQ(map.getSafeZones().size(), 1u);
    EXPECT_EQ(map.terrainAt(25, 25), TerrainCode::COBBLE);

    nlohmann::json data = nlohmann::json::parse(map.toJson());
    ASSERT_TRUE(data.contains("obstacles"));
    EXPECT_FALSE(data["obstacles"].empty());
}

TEST(CityStampTest, ClearRemovesCity) {
    EditorMap map(60, 60, 32, "5108.png", 32);
    applyCityPrefab(map, 25, 25, "Ullathorpe");
    clearCity(map, 25, 25);

    EXPECT_TRUE(map.getCitizens().empty());
    EXPECT_TRUE(map.getSafeZones().empty());
    EXPECT_EQ(map.terrainAt(25, 25), TerrainCode::GRASS);
    EXPECT_EQ(map.tileAt(28, 25), 0);  // pared de la iglesia borrada (origin 25 + dx 3)
}
