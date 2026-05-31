#include <gtest/gtest.h>

#include "CharRect.h"

TEST(BitmapFontTest, MapsCapitalAToExpectedCell) {
    CharRect r = getCharRect('A');
    EXPECT_EQ(r.x, 1 * 128);
    EXPECT_EQ(r.y, 4 * 128);
    EXPECT_EQ(r.w, 128);
    EXPECT_EQ(r.h, 128);
}

TEST(BitmapFontTest, MapsZeroToExpectedCell) {
    CharRect r = getCharRect('0');
    EXPECT_EQ(r.x, 0);
    EXPECT_EQ(r.y, 3 * 128);
}

TEST(BitmapFontTest, MapsSpaceToExpectedCell) {
    CharRect r = getCharRect(' ');
    EXPECT_EQ(r.x, 0);
    EXPECT_EQ(r.y, 2 * 128);
}

TEST(BitmapFontTest, MapsLowercaseAToExpectedCell) {
    CharRect r = getCharRect('a');
    EXPECT_EQ(r.x, 1 * 128);
    EXPECT_EQ(r.y, 6 * 128);
}
