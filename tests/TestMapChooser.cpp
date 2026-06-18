#include <gtest/gtest.h>

#include "MapChooser.h"

TEST(MapChooserTest, BuildsEntriesWithDisplayNamesAndAppendsNewMap) {
    std::vector<std::string> paths = {"maps/defaultMap.json", "maps/ciudad.json"};
    std::vector<MapEntry> entries = mapEntriesFrom(paths);

    ASSERT_EQ(entries.size(), 3u);
    EXPECT_EQ(entries[0].displayName, "defaultMap");
    EXPECT_EQ(entries[0].path, "maps/defaultMap.json");
    EXPECT_FALSE(entries[0].isNew);
    EXPECT_EQ(entries[1].displayName, "ciudad");
    EXPECT_FALSE(entries[1].isNew);
    EXPECT_EQ(entries[2].displayName, "Nuevo mapa");
    EXPECT_TRUE(entries[2].isNew);
    EXPECT_EQ(entries[2].path, "");
}

TEST(MapChooserTest, EmptyDirStillOffersNewMap) {
    std::vector<MapEntry> entries = mapEntriesFrom({});
    ASSERT_EQ(entries.size(), 1u);
    EXPECT_TRUE(entries[0].isNew);
}

TEST(MapChooserTest, MapPathForNameTrimsAndAddsPrefixAndExtension) {
    EXPECT_EQ(mapPathForName("ciudad"), "maps/ciudad.json");
    EXPECT_EQ(mapPathForName("  bosque  "), "maps/bosque.json");
}

TEST(MapChooserTest, NewMapErrorRejectsEmpty) {
    EXPECT_NE(newMapError("   ", {}), "");
}

TEST(MapChooserTest, NewMapErrorRejectsInvalidChars) {
    EXPECT_NE(newMapError("hola mundo", {}), "");
    EXPECT_NE(newMapError("a/b", {}), "");
    EXPECT_NE(newMapError("mapa.json", {}), "");
}

TEST(MapChooserTest, NewMapErrorRejectsCollisionIncludingDefault) {
    std::vector<std::string> existing = {"maps/defaultMap.json", "maps/ciudad.json"};
    EXPECT_NE(newMapError("defaultMap", existing), "");
    EXPECT_NE(newMapError("ciudad", existing), "");
}

TEST(MapChooserTest, NewMapErrorAcceptsValidUniqueName) {
    std::vector<std::string> existing = {"maps/defaultMap.json"};
    EXPECT_EQ(newMapError("bosque_2", existing), "");
}
