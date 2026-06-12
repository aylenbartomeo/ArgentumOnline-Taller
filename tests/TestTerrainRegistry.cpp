#include <gtest/gtest.h>

#include "OverlayRegistry.h"
#include "TerrainRegistry.h"

TEST(TerrainRegistryTest, HasOneEntryPerCode) {
    const auto& reg = getTerrainRegistry();
    EXPECT_GE(reg.size(), 5u);
    EXPECT_EQ(reg[TerrainCode::GRASS].name, "pasto");
    EXPECT_EQ(reg[TerrainCode::WATER].name, "agua");
    EXPECT_EQ(reg[TerrainCode::SAND].name, "arena");
}

TEST(TerrainRegistryTest, EveryDefHasNonEmptySheet) {
    for (const auto& def : getTerrainRegistry()) {
        EXPECT_FALSE(def.sheet.empty());
    }
}

TEST(OverlayRegistryTest, NewTilesPointToExpectedEntries) {
    const auto& reg = getOverlayRegistry();
    ASSERT_GE(reg.size(), 30u);
    EXPECT_EQ(reg[OverlayTile::WALL - 1].name, "Pared");
    EXPECT_TRUE(reg[OverlayTile::WALL - 1].solid);
    EXPECT_EQ(reg[OverlayTile::ALTAR - 1].name, "Altar");
    EXPECT_FALSE(reg[OverlayTile::ALTAR - 1].solid);
    EXPECT_EQ(reg[OverlayTile::BOVEDA - 1].name, "Boveda");
}
