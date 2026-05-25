#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "TileMap.h"

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
            },
            std::runtime_error);
}
