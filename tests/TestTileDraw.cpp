#include <gtest/gtest.h>

#include "../client/src/rendering/TileDraw.h"

TEST(TileDrawTest, AnchorsTileToBottomLeftOfCell) {
    TileRect r = tileDestRect(2, 3, 64, 96, 32, 10, 20);
    EXPECT_EQ(r.x, 2 * 32 - 10);
    EXPECT_EQ(r.y, (3 + 1) * 32 - 96 - 20);
    EXPECT_EQ(r.w, 64);
    EXPECT_EQ(r.h, 96);
}

TEST(TileDrawTest, ThirtyTwoTileIsCellAligned) {
    TileRect r = tileDestRect(1, 1, 32, 32, 32, 0, 0);
    EXPECT_EQ(r.x, 32);
    EXPECT_EQ(r.y, 32);
}
