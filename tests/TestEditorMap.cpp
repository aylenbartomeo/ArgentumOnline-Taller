#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "EditorMap.h"
#include "OverlayRegistry.h"

namespace {
int goldOverlayIndex() {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    for (size_t i = 0; i < registry.size(); ++i) {
        if (registry[i].itemId == 1) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

nlohmann::json builderStyleJson() {
    nlohmann::json m;
    m["width"] = 6;
    m["height"] = 6;
    m["tileSize"] = 32;
    m["tileset"] = "5108.png";
    m["tilesetCols"] = 32;
    m["spawn"] = {{"x", 2}, {"y", 3}};
    m["ground"] = std::vector<std::vector<int>>(6, std::vector<int>(6, 108));
    m["ground2"] = std::vector<std::vector<int>>(6, std::vector<int>(6, 0));
    m["decoration"] = std::vector<std::vector<int>>(6, std::vector<int>(6, 0));
    m["roofs"] = std::vector<std::vector<int>>(6, std::vector<int>(6, 0));
    m["indoor"] = std::vector<std::vector<int>>(6, std::vector<int>(6, 0));
    m["ground"][1][2] = 109;
    m["decoration"][4][1] = 201;
    m["roofs"][4][1] = 204;
    m["indoor"][3][1] = 1;
    m["obstacles"] = {{{"x", 2}, {"y", 1}}, {{"x", 1}, {"y", 4}}};
    m["npcs"] = {{{"type", "priest"}, {"x", 1}, {"y", 3}}};
    m["monsters"] = {{{"type", "goblin"}, {"x", 5}, {"y", 5}}};
    m["safeZones"] = {{{"name", "Pueblo"}, {"x", 0}, {"y", 0}, {"width", 4}, {"height", 5}}};
    m["items"] = {{{"id", 1}, {"x", 3}, {"y", 3}, {"amount", 7}}};
    return m;
}
}  // namespace

TEST(EditorMapTest, NewMapStartsWithGrassAndNoContent) {
    EditorMap map(4, 5, 32, "5108.png", 32);
    EXPECT_EQ(map.getWidth(), 4);
    EXPECT_EQ(map.getHeight(), 5);
    EXPECT_EQ(map.getGround()[0][0], 108);
    EXPECT_EQ(map.getGround2()[4][3], 0);
    EXPECT_EQ(map.getDecoration()[2][2], 0);
    EXPECT_TRUE(map.getCitizens().empty());
    EXPECT_TRUE(map.getMonsters().empty());
    EXPECT_TRUE(map.getSafeZones().empty());
    EXPECT_EQ(map.getSpawn().x, 0);
    EXPECT_EQ(map.getSpawn().y, 0);
}

TEST(EditorMapTest, RoundTripPreservesEverything) {
    nlohmann::json original = builderStyleJson();
    EditorMap map(original.dump());
    nlohmann::json saved = nlohmann::json::parse(map.toJson());

    EXPECT_EQ(saved["width"], original["width"]);
    EXPECT_EQ(saved["height"], original["height"]);
    EXPECT_EQ(saved["tileSize"], original["tileSize"]);
    EXPECT_EQ(saved["tileset"], original["tileset"]);
    EXPECT_EQ(saved["tilesetCols"], original["tilesetCols"]);
    EXPECT_EQ(saved["spawn"], original["spawn"]);
    EXPECT_EQ(saved["ground"], original["ground"]);
    EXPECT_EQ(saved["ground2"], original["ground2"]);
    EXPECT_EQ(saved["decoration"], original["decoration"]);
    EXPECT_EQ(saved["roofs"], original["roofs"]);
    EXPECT_EQ(saved["indoor"], original["indoor"]);
    EXPECT_EQ(saved["obstacles"], original["obstacles"]);
    EXPECT_EQ(saved["npcs"], original["npcs"]);
    EXPECT_EQ(saved["monsters"], original["monsters"]);
    EXPECT_EQ(saved["safeZones"], original["safeZones"]);
    EXPECT_EQ(saved["items"], original["items"]);
}

TEST(EditorMapTest, MissingLayersDefaultToGrassAndZeros) {
    nlohmann::json m;
    m["width"] = 4;
    m["height"] = 4;
    m["tileSize"] = 32;
    m["tileset"] = "5108.png";
    m["tilesetCols"] = 32;
    EditorMap map(m.dump());

    EXPECT_EQ(map.getGround()[0][0], 108);
    EXPECT_EQ(map.getGround2()[0][0], 0);
    EXPECT_EQ(map.getDecoration()[3][3], 0);
    EXPECT_EQ(map.getSpawn().x, 0);
}

TEST(EditorMapTest, LoadJsonWithWrongRowCountThrows) {
    nlohmann::json m;
    m["width"] = 4;
    m["height"] = 4;
    m["tileSize"] = 32;
    m["tileset"] = "5108.png";
    m["tilesetCols"] = 32;
    m["ground"] = std::vector<std::vector<int>>(2, std::vector<int>(4, 108));
    EXPECT_THROW(EditorMap map(m.dump()), std::runtime_error);
}

TEST(EditorMapTest, SetAndGetSpawn) {
    EditorMap map(4, 4, 32, "5108.png", 32);
    map.setSpawn(2, 3);
    EXPECT_EQ(map.getSpawn().x, 2);
    EXPECT_EQ(map.getSpawn().y, 3);
}

TEST(EditorMapTest, IsBlockedMatchesObstacles) {
    EditorMap map(4, 4, 32, "5108.png", 32);
    EXPECT_FALSE(map.isBlocked(1, 1));
    map.addObstacle(1, 1);
    EXPECT_TRUE(map.isBlocked(1, 1));
    EXPECT_FALSE(map.isBlocked(2, 1));
}

TEST(EditorMapTest, PaintGoldStacksAmount) {
    EditorMap map(4, 4, 32, "5108.png", 32);
    int gold = goldOverlayIndex();
    ASSERT_GE(gold, 0);
    map.paintItem(2, 2, gold);
    map.paintItem(2, 2, gold);
    const PlacedItem* item = map.itemAt(2, 2);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->amount, 2);

    map.removeItemAt(2, 2);
    EXPECT_EQ(map.itemAt(2, 2), nullptr);
}

TEST(EditorMapTest, PaintNonStackableItemStaysAtOne) {
    EditorMap map(4, 4, 32, "5108.png", 32);
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    int sword = -1;
    for (size_t i = 0; i < registry.size(); ++i) {
        if (registry[i].itemId != 0 && !registry[i].stackable) {
            sword = static_cast<int>(i);
            break;
        }
    }
    ASSERT_GE(sword, 0);
    map.paintItem(1, 1, sword);
    map.paintItem(1, 1, sword);
    const PlacedItem* item = map.itemAt(1, 1);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->amount, 1);
}

TEST(EditorMapTest, SavedGoldKeepsIdAndAmount) {
    EditorMap map(4, 4, 32, "5108.png", 32);
    int gold = goldOverlayIndex();
    ASSERT_GE(gold, 0);
    map.paintItem(3, 1, gold);
    map.paintItem(3, 1, gold);
    map.paintItem(3, 1, gold);

    nlohmann::json saved = nlohmann::json::parse(map.toJson());
    ASSERT_EQ(saved["items"].size(), 1u);
    EXPECT_EQ(saved["items"][0]["id"], 1);
    EXPECT_EQ(saved["items"][0]["x"], 3);
    EXPECT_EQ(saved["items"][0]["y"], 1);
    EXPECT_EQ(saved["items"][0]["amount"], 3);
}

TEST(EditorMapTest, UnknownItemIdsAreKeptVerbatim) {
    nlohmann::json m = builderStyleJson();
    m["items"].push_back({{"id", 99999}, {"x", 5}, {"y", 0}, {"amount", 2}});
    EditorMap map(m.dump());
    nlohmann::json saved = nlohmann::json::parse(map.toJson());
    bool kept = false;
    for (const auto& item: saved["items"]) {
        if (item["id"] == 99999) {
            kept = true;
        }
    }
    EXPECT_TRUE(kept);
}

TEST(EditorMapTest, AddRemoveEntitiesAtPosition) {
    EditorMap map(4, 4, 32, "5108.png", 32);
    map.addCitizen("priest", 1, 1);
    map.addMonster("goblin", 1, 1);
    map.addCitizen("banker", 2, 2);
    map.removeEntitiesAt(1, 1);
    ASSERT_EQ(map.getCitizens().size(), 1u);
    EXPECT_EQ(map.getCitizens()[0].type, "banker");
    EXPECT_TRUE(map.getMonsters().empty());
}

TEST(OverlayRegistryTest, ContainsStackableGold) {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    bool found = false;
    for (const OverlayDef& def: registry) {
        if (def.itemId == 1 && def.stackable) {
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

TEST(EditorMapDungeonTest, AddAndRemoveDungeon) {
    EditorMap map(50, 50, 32, "5108.png", 32);
    map.addDungeon(11, 21, 14, 14);
    ASSERT_EQ(map.getDungeons().size(), 1u);
    EXPECT_EQ(map.getDungeons()[0].x, 11);
    EXPECT_EQ(map.getDungeons()[0].y, 21);
    EXPECT_EQ(map.getDungeons()[0].width, 14);
    EXPECT_EQ(map.getDungeons()[0].height, 14);
    map.removeDungeonAt(11, 21);
    EXPECT_TRUE(map.getDungeons().empty());
}

TEST(EditorMapDungeonTest, DungeonsSurviveJsonRoundTrip) {
    EditorMap map(50, 50, 32, "5108.png", 32);
    map.addDungeon(11, 21, 14, 14);
    EditorMap loaded(map.toJson());
    ASSERT_EQ(loaded.getDungeons().size(), 1u);
    EXPECT_EQ(loaded.getDungeons()[0].x, 11);
    EXPECT_EQ(loaded.getDungeons()[0].height, 14);
}

TEST(EditorMapDungeonTest, SetItemStoresGivenAmount) {
    EditorMap map(50, 50, 32, "5108.png", 32);
    map.setItem(5, 6, goldOverlayIndex(), 5000);
    const PlacedItem* item = map.itemAt(5, 6);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->overlayIndex, goldOverlayIndex());
    EXPECT_EQ(item->amount, 5000);
}
