#include <gtest/gtest.h>

#include "server/src/model/map/SafeZoneLayer.h"

TEST(SafeZoneLayerTest, IsSafeZone_inside_and_outside) {
    SafeZoneLayer layer;
    layer.addZone("Ullathorpe", 10, 10, 20, 20);

    // Inside
    EXPECT_TRUE(layer.isSafeZone(15.0f, 15.0f));
    EXPECT_TRUE(layer.isSafeZone(10.0f, 10.0f));
    EXPECT_TRUE(layer.isSafeZone(30.0f, 30.0f));

    // Outside
    EXPECT_FALSE(layer.isSafeZone(9.9f, 15.0f));
    EXPECT_FALSE(layer.isSafeZone(15.0f, 30.1f));
}

TEST(SafeZoneLayerTest, MultipleSafeZones_work) {
    SafeZoneLayer layer;
    layer.addZone("Zone1", 0, 0, 10, 10);
    layer.addZone("Zone2", 50, 50, 10, 10);

    EXPECT_TRUE(layer.isSafeZone(5.0f, 5.0f));
    EXPECT_TRUE(layer.isSafeZone(55.0f, 55.0f));
    EXPECT_FALSE(layer.isSafeZone(25.0f, 25.0f));
}

TEST(SafeZoneLayerTest, GetZoneName_returns_correct_name) {
    SafeZoneLayer layer;
    layer.addZone("Nix", 100, 100, 20, 20);

    EXPECT_EQ(layer.getZoneName(110.0f, 110.0f), "Nix");
    EXPECT_EQ(layer.getZoneName(0.0f, 0.0f), "");
}
