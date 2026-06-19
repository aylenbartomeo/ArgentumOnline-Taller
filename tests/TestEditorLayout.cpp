#include <gtest/gtest.h>

#include "EditorLayout.h"

namespace {
int cx(LayoutRect r) { return r.x + r.w / 2; }
int cy(LayoutRect r) { return r.y + r.h / 2; }
}  // namespace

TEST(EditorLayoutTest, PrincipalNavButtonsGoToTheirScreens) {
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 1275, 866), Region::GO_TERRENO);
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 1275, 536), Region::GO_ITEMS);
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 1275, 696), Region::GO_MONSTRUOS);
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 1275, 365), Region::GO_CIUDADANOS);
}

TEST(EditorLayoutTest, NavButtonsDoNotNavigateFromSubScreens) {
    EXPECT_NE(regionAtClick(Screen::TERRENO, 1275, 866), Region::GO_TERRENO);
}

TEST(EditorLayoutTest, BackOnlyOnSubScreens) {
    EXPECT_EQ(regionAtClick(Screen::TERRENO, 80, 942), Region::BACK);
    EXPECT_NE(regionAtClick(Screen::PRINCIPAL, 80, 942), Region::BACK);
}

TEST(EditorLayoutTest, PanelBackOnNonTerrenoSubScreens) {
    EXPECT_EQ(regionAtClick(Screen::ITEMS, 1145, 1003), Region::BACK);
    EXPECT_EQ(regionAtClick(Screen::MONSTRUOS, 1145, 1003), Region::BACK);
    EXPECT_EQ(regionAtClick(Screen::CIUDADANOS, 1145, 1003), Region::BACK);
    EXPECT_NE(regionAtClick(Screen::TERRENO, 1145, 1003), Region::BACK);
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
    EXPECT_EQ(mockupFile(Screen::TERRENO), "EditorTerrenos.png");
    EXPECT_EQ(mockupFile(Screen::ITEMS), "EditorItems.png");
    EXPECT_EQ(mockupFile(Screen::MONSTRUOS), "EditorMonstruos.png");
    EXPECT_EQ(mockupFile(Screen::CIUDADANOS), "EditorCiudadanos.png");
    EXPECT_EQ(screenForRegion(Region::GO_TERRENO), Screen::TERRENO);
    EXPECT_EQ(screenForRegion(Region::GO_CIUDADANOS), Screen::CIUDADANOS);
}

TEST(EditorLayoutTest, SpawnOnEveryScreen) {
    EXPECT_EQ(regionAtClick(Screen::PRINCIPAL, 267, 126), Region::SPAWN);
    EXPECT_EQ(regionAtClick(Screen::MONSTRUOS, 267, 126), Region::SPAWN);
}

TEST(EditorLayoutTest, GomaAndSpawnRectsAreExposed) {
    LayoutRect g = gomaRect();
    EXPECT_EQ(g.x, 48);
    EXPECT_EQ(g.y, 48);
    EXPECT_EQ(g.w, 152);
    EXPECT_EQ(g.h, 156);
    LayoutRect s = spawnRect();
    EXPECT_EQ(s.x, 200);
    EXPECT_EQ(s.y, 48);
    EXPECT_EQ(s.w, 135);
    EXPECT_EQ(s.h, 156);
}

TEST(EditorLayoutTest, GuardarRectIsExposed) {
    LayoutRect g = guardarRect();
    EXPECT_EQ(g.x, 1118);
    EXPECT_EQ(g.y, 148);
    EXPECT_EQ(g.w, 125);
    EXPECT_EQ(g.h, 95);
}

TEST(EditorLayoutTest, MapasRectIsExposed) {
    LayoutRect m = mapasRect();
    EXPECT_EQ(m.x, 1284);
    EXPECT_EQ(m.y, 80);
    EXPECT_EQ(m.w, 85);
    EXPECT_EQ(m.h, 120);
}
