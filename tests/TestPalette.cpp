#include <gtest/gtest.h>

#include "Palette.h"

TEST(PaletteTest, SelectsFirstTileByDefault) {
    Palette palette(600, 190, 20, 9, 132);
    EXPECT_EQ(palette.getSelectedTile(), 0);
}

TEST(PaletteTest, ScrollClampsToBounds) {
    Palette palette(600, 190, 20, 9, 132);
    palette.scroll(-5);
    EXPECT_EQ(palette.getScrollRow(), 0);
    palette.scroll(1000);
    EXPECT_EQ(palette.getScrollRow(), (132 + 8) / 9 - 1);
}
