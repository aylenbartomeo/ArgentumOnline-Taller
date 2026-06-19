#include <gtest/gtest.h>

#include "EditorLayout.h"

namespace {
int cx(LayoutRect r) { return r.x + r.w / 2; }
int cy(LayoutRect r) { return r.y + r.h / 2; }
}  // namespace

TEST(EditorLayoutTest, PrincipalNavButtonsGoToTheirScreens) {
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 1275, 752), Region::GO_TERRENO);
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 1275, 412), Region::GO_ITEMS);
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 1275, 582), Region::GO_MONSTRUOS);
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 1275, 242), Region::GO_CIUDADANOS);
}

TEST(EditorLayoutTest, NavButtonsDoNotNavigateFromSubScreens) {
    EXPECT_NE(regionAtClick(Screen::TERRENO, 1275, 752), Region::GO_TERRENO);
}

TEST(EditorLayoutTest, BackOnlyOnSubScreens) {
    EXPECT_EQ(regionAtClick(Screen::TERRENO, 80, 942), Region::BACK);
    EXPECT_NE(regionAtClick(Screen::PRINCIPAL, 80, 942), Region::BACK);
}

TEST(EditorLayoutTest, CanvasClickInsideCanvas) {
    LayoutRect c = canvasRect();
    EXPECT_EQ(regionAtClick(Screen::TERRENO, cx(c), cy(c)), Region::CANVAS);
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, cx(c), cy(c)), Region::CANVAS);
}

TEST(EditorLayoutTest, GomaOnEveryScreen) {
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 124, 126), Region::GOMA);
    EXPECT_EQ(regionAtClick(Screen::MONSTRUOS, 124, 126), Region::GOMA);
}

TEST(EditorLayoutTest, OutsideEverythingIsNone) {
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 1445, 1080), Region::NONE);
}

TEST(EditorLayoutTest, MockupFilesAndScreenForRegion) {
    EXPECT_EQ(mockupFile(Screen::PRINCIPAL), "EditorPrincipal.png");
    EXPECT_EQ(mockupFile(Screen::TERRENO), "EditorTerreno.png");
    EXPECT_EQ(mockupFile(Screen::ITEMS), "EditorItems.png");
    EXPECT_EQ(mockupFile(Screen::MONSTRUOS), "EditorMonstruos.png");
    EXPECT_EQ(mockupFile(Screen::CIUDADANOS), "EditorCiudadanos.png");
    EXPECT_EQ(screenForRegion(Region::GO_TERRENO), Screen::TERRENO);
    EXPECT_EQ(screenForRegion(Region::GO_CIUDADANOS), Screen::CIUDADANOS);
}
