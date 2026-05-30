#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "EditorMap.h"

TEST(EditorMapTest, NewMapIsEmptyWithGivenDimensions) {
    EditorMap map(4, 3, 16, "tilemap_packed.png", 12);
    EXPECT_EQ(map.getWidth(), 4);
    EXPECT_EQ(map.getHeight(), 3);
    EXPECT_EQ(map.getTileSize(), 16);
    EXPECT_EQ(map.getTilesetCols(), 12);
    EXPECT_EQ(map.getTileset(), "tilemap_packed.png");
    EXPECT_EQ(map.tileAt(0, 0), 0);
    EXPECT_EQ(map.tileAt(3, 2), 0);
}

TEST(EditorMapTest, SetAndGetTile) {
    EditorMap map(4, 3, 16, "tilemap_packed.png", 12);
    map.setTile(2, 1, 25);
    EXPECT_EQ(map.tileAt(2, 1), 25);
}

TEST(EditorMapTest, NewMapSpawnDefaultsToZero) {
    EditorMap map(4, 3, 16, "tilemap_packed.png", 12);
    EXPECT_EQ(map.getSpawn().x, 0);
    EXPECT_EQ(map.getSpawn().y, 0);
}

TEST(EditorMapTest, SetAndGetSpawn) {
    EditorMap map(4, 3, 16, "tilemap_packed.png", 12);
    map.setSpawn(2, 1);
    EXPECT_EQ(map.getSpawn().x, 2);
    EXPECT_EQ(map.getSpawn().y, 1);
}

TEST(EditorMapTest, ToJsonRoundTrip) {
    EditorMap original(3, 2, 16, "tilemap_packed.png", 12);
    original.setTile(0, 0, 5);
    original.setTile(2, 1, 9);
    original.setSpawn(1, 1);

    EditorMap reloaded(original.toJson());

    EXPECT_EQ(reloaded.getWidth(), 3);
    EXPECT_EQ(reloaded.getHeight(), 2);
    EXPECT_EQ(reloaded.getTileSize(), 16);
    EXPECT_EQ(reloaded.getTilesetCols(), 12);
    EXPECT_EQ(reloaded.getTileset(), "tilemap_packed.png");
    EXPECT_EQ(reloaded.tileAt(0, 0), 5);
    EXPECT_EQ(reloaded.tileAt(2, 1), 9);
    EXPECT_EQ(reloaded.getSpawn().x, 1);
    EXPECT_EQ(reloaded.getSpawn().y, 1);
}

TEST(EditorMapTest, LoadJsonWithoutSpawnDefaultsToZero) {
    std::string json = R"({
        "tileSize": 16,
        "tileset": "tilemap_packed.png",
        "tilesetCols": 12,
        "width": 2,
        "height": 2,
        "tiles": [[0, 0], [0, 0]]
    })";
    EditorMap map(json);
    EXPECT_EQ(map.getSpawn().x, 0);
    EXPECT_EQ(map.getSpawn().y, 0);
}

TEST(EditorMapTest, LoadJsonWithWrongRowCountThrows) {
    std::string json = R"({
        "tileSize": 16,
        "tileset": "tilemap_packed.png",
        "tilesetCols": 12,
        "width": 2,
        "height": 3,
        "tiles": [[0, 0], [0, 0]]
    })";
    EXPECT_THROW(EditorMap map(json), std::runtime_error);
}

TEST(EditorMapTest, ExposesSafeZonesParsedFromJson) {
    std::string json = R"({
        "tileSize": 32,
        "tileset": "5108.png",
        "tilesetCols": 32,
        "width": 2,
        "height": 2,
        "tiles": [[0, 0], [0, 0]],
        "safeZones": [
            { "name": "Ullathorpe", "x": 4, "y": 5, "width": 6, "height": 3 }
        ]
    })";
    EditorMap map(json);
    ASSERT_EQ(map.getSafeZones().size(), 1u);
    EXPECT_EQ(map.getSafeZones()[0].x, 4);
    EXPECT_EQ(map.getSafeZones()[0].y, 5);
    EXPECT_EQ(map.getSafeZones()[0].width, 6);
    EXPECT_EQ(map.getSafeZones()[0].height, 3);
}

TEST(EditorMapTest, RetainsSafeZonesAndNPCsOnLoadAndSave) {
    std::string json = R"({
        "tileSize": 16,
        "tileset": "tilemap_packed.png",
        "tilesetCols": 12,
        "width": 2,
        "height": 2,
        "tiles": [[0, 0], [0, 0]],
        "safeZones": [
            { "name": "Ullathorpe", "x": 6, "y": 5, "width": 8, "height": 5 }
        ],
        "npcs": [
            { "type": "merchant", "x": 8, "y": 7 }
        ]
    })";

    EditorMap map(json);
    std::string outJson = map.toJson();

    nlohmann::json parsedOut = nlohmann::json::parse(outJson);

    ASSERT_TRUE(parsedOut.contains("safeZones"));
    EXPECT_EQ(parsedOut["safeZones"][0]["name"], "Ullathorpe");

    ASSERT_TRUE(parsedOut.contains("npcs"));
    EXPECT_EQ(parsedOut["npcs"][0]["type"], "merchant");
}

TEST(EditorMapTest, ResizeGrowFillsWithZero) {
    EditorMap map(2, 2, 16, "tilemap_packed.png", 12);
    map.setTile(0, 0, 7);
    map.resize(3, 3);
    EXPECT_EQ(map.getWidth(), 3);
    EXPECT_EQ(map.getHeight(), 3);
    EXPECT_EQ(map.tileAt(0, 0), 7);
    EXPECT_EQ(map.tileAt(2, 2), 0);
}

TEST(EditorMapTest, ResizeShrinkClampsSpawn) {
    EditorMap map(4, 4, 16, "tilemap_packed.png", 12);
    map.setSpawn(3, 3);
    map.resize(2, 2);
    EXPECT_LT(map.getSpawn().x, 2);
    EXPECT_LT(map.getSpawn().y, 2);
}
