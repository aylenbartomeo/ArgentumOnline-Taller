#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "../client/src/rendering/TileMap.h"

TEST(TileMapTest, ParsesValidMap) {
    std::string json = R"({
        "tileSize": 16,
        "tileset": "tilemap_packed.png",
        "tilesetCols": 12,
        "width": 2,
        "height": 2,
        "tiles": [[0, 1], [2, 3]]
    })";

    TileMap map(json);

    EXPECT_EQ(map.getWidth(), 2);
    EXPECT_EQ(map.getHeight(), 2);
    EXPECT_EQ(map.getTileSize(), 16);
    EXPECT_EQ(map.getTilesetCols(), 12);
    EXPECT_EQ(map.getTileset(), "tilemap_packed.png");

    EXPECT_EQ(map.tileAt(0, 0), 0);
    EXPECT_EQ(map.tileAt(1, 0), 1);
    EXPECT_EQ(map.tileAt(0, 1), 2);
    EXPECT_EQ(map.tileAt(1, 1), 3);
}

TEST(TileMapTest, ThrowsWhenTilesDoNotMatchDimensions) {
    EXPECT_THROW(
            {
                std::string json = R"({
            "tileSize": 16,
            "tileset": "x.png",
            "tilesetCols": 12,
            "width": 3,
            "height": 2,
            "tiles": [[0, 1], [2, 3]]
        })";
                TileMap map(json);
                (void)map;
            },
            std::runtime_error);
}

TEST(TileMapTest, ParsesSafeZones) {
    std::string json = R"({
        "tileSize": 32,
        "tileset": "5108.png",
        "tilesetCols": 32,
        "width": 2,
        "height": 2,
        "tiles": [[0, 0], [0, 0]],
        "safeZones": [
            { "name": "Ullathorpe", "x": 5, "y": 6, "width": 8, "height": 4 },
            { "name": "Nix", "x": 20, "y": 21, "width": 3, "height": 2 }
        ]
    })";

    TileMap map(json);

    ASSERT_EQ(map.getSafeZones().size(), 2u);
    EXPECT_EQ(map.getSafeZones()[0].x, 5);
    EXPECT_EQ(map.getSafeZones()[0].y, 6);
    EXPECT_EQ(map.getSafeZones()[0].width, 8);
    EXPECT_EQ(map.getSafeZones()[0].height, 4);
    EXPECT_EQ(map.getSafeZones()[1].x, 20);
}

TEST(TileMapTest, NoSafeZonesWhenAbsent) {
    std::string json = R"({
        "tileSize": 32,
        "tileset": "5108.png",
        "tilesetCols": 32,
        "width": 1,
        "height": 1,
        "tiles": [[0]]
    })";
    TileMap map(json);
    EXPECT_TRUE(map.getSafeZones().empty());
}

TEST(TileMapTest, ParsesCitizens) {
    std::string json = R"({
        "tileSize": 32,
        "tileset": "5108.png",
        "tilesetCols": 32,
        "width": 2,
        "height": 2,
        "tiles": [[0, 0], [0, 0]],
        "npcs": [
            { "type": "merchant", "x": 1, "y": 1 },
            { "type": "priest", "x": 0, "y": 1 }
        ]
    })";
    TileMap map(json);
    ASSERT_EQ(map.getCitizens().size(), 2u);
    EXPECT_EQ(map.getCitizens()[0].type, "merchant");
    EXPECT_EQ(map.getCitizens()[0].x, 1);
    EXPECT_EQ(map.getCitizens()[0].y, 1);
    EXPECT_EQ(map.getCitizens()[1].type, "priest");
}

TEST(TileMapTest, NoCitizensWhenAbsent) {
    std::string json = R"({
        "tileSize": 32,
        "tileset": "5108.png",
        "tilesetCols": 32,
        "width": 1,
        "height": 1,
        "tiles": [[0]]
    })";
    TileMap map(json);
    EXPECT_TRUE(map.getCitizens().empty());
}

TEST(TileMapTest, ParsesTerrainLayer) {
    std::string json = R"({
        "tileSize": 32, "tileset": "5108.png", "tilesetCols": 32,
        "width": 2, "height": 2,
        "tiles": [[0, 0], [0, 0]],
        "terrain": [[0, 1], [2, 3]]
    })";

    TileMap map(json);

    EXPECT_EQ(map.terrainAt(0, 0), 0);
    EXPECT_EQ(map.terrainAt(1, 0), 1);
    EXPECT_EQ(map.terrainAt(0, 1), 2);
    EXPECT_EQ(map.terrainAt(1, 1), 3);
}

TEST(TileMapTest, DefaultsTerrainToGrassWhenAbsent) {
    std::string json = R"({
        "tileSize": 32, "tileset": "5108.png", "tilesetCols": 32,
        "width": 2, "height": 2,
        "tiles": [[0, 0], [0, 0]]
    })";

    TileMap map(json);

    EXPECT_EQ(map.terrainAt(0, 0), 0);
    EXPECT_EQ(map.terrainAt(1, 1), 0);
}

TEST(TileMapTest, ParsesLayerGrids) {
    std::string json = R"({
        "tileSize": 32, "tileset": "x.png", "tilesetCols": 32,
        "width": 2, "height": 2,
        "tiles": [[0, 0], [0, 0]],
        "ground": [[1, 2], [3, 4]],
        "decoration": [[0, 5], [0, 0]],
        "indoor": [[0, 1], [0, 0]]
    })";

    TileMap map(json);

    EXPECT_EQ(map.getGround()[0][0], 1);
    EXPECT_EQ(map.getGround()[1][1], 4);
    EXPECT_EQ(map.getDecoration()[0][1], 5);
    EXPECT_EQ(map.getIndoor()[0][1], 1);
}

TEST(TileMapTest, LayersDefaultToEmpty) {
    std::string json = R"({
        "tileSize": 32, "tileset": "x.png", "tilesetCols": 32,
        "width": 2, "height": 2,
        "tiles": [[0, 0], [0, 0]]
    })";

    TileMap map(json);

    EXPECT_EQ(map.getGround()[1][1], 0);
    EXPECT_EQ(map.getRoofs()[0][0], 0);
}
