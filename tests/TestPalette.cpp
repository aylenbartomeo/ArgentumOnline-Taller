#include <gtest/gtest.h>

#include "Palette.h"

TEST(PaletteTest, SelectsFirstTileByDefault) {
    Palette palette(600, 190, 20, 9, 132);
    EXPECT_EQ(palette.getSelectedTile(), 0);
}

TEST(PaletteTest, ClickSelectsTileByPosition) {
    Palette palette(600, 190, 20, 9, 132);
    palette.selectFromClick(600 + 20 * 2, 190 + 20 * 1);
    EXPECT_EQ(palette.getSelectedTile(), 9 + 2);
}

TEST(PaletteTest, ClickOutsideToTheLeftIsIgnored) {
    Palette palette(600, 190, 20, 9, 132);
    palette.selectFromClick(100, 190);
    EXPECT_EQ(palette.getSelectedTile(), 0);
}

TEST(PaletteTest, ClickBeyondTileCountIsIgnored) {
    Palette palette(600, 190, 20, 2, 3);
    palette.selectFromClick(600 + 20 * 1, 190 + 20 * 5);
    EXPECT_EQ(palette.getSelectedTile(), 0);
}
