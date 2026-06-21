#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "SpawnCatalogs.h"

TEST(SpawnCatalogsTest, MonsterCatalogHasSixEntries) { EXPECT_EQ(getMonsterCatalog().size(), 6u); }

TEST(SpawnCatalogsTest, MonsterCatalogReferencesExpectedSheets) {
    const auto& catalog = getMonsterCatalog();
    auto find = [&](const std::string& type) {
        auto it = std::find_if(catalog.begin(), catalog.end(),
                               [&type](const auto& entry) { return entry.type == type; });
        return it != catalog.end() ? it->sheet : std::string("");
    };
    EXPECT_EQ(find("goblin"), "characters/monsters/goblin1.png");
    EXPECT_EQ(find("orc"), "characters/monsters/orco1.png");
    EXPECT_EQ(find("zombie"), "characters/monsters/zombie1.png");
    EXPECT_EQ(find("spider"), "characters/monsters/spider1.png");
    EXPECT_EQ(find("golem"), "characters/monsters/golem1.png");
    EXPECT_EQ(find("skeleton"), "characters/monsters/skeleton1.png");
}

TEST(SpawnCatalogsTest, CitizenCatalogHasMerchantBankerPriest) {
    const auto& catalog = getCitizenCatalog();
    ASSERT_EQ(catalog.size(), 3u);
    auto find = [&](const std::string& type) {
        auto it = std::find_if(catalog.begin(), catalog.end(),
                               [&type](const auto& entry) { return entry.type == type; });
        return it != catalog.end() ? it->sheet : std::string("");
    };
    EXPECT_EQ(find("merchant"), "characters/npc-merchant.png");
    EXPECT_EQ(find("banker"), "characters/npc-banker.png");
    EXPECT_EQ(find("priest"), "characters/npc-priest.png");
}

TEST(SpawnCatalogsTest, MonsterEntryForKnownTypeReturnsIt) {
    const MonsterCatalogEntry* entry = monsterEntryFor("goblin");
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->type, "goblin");
}

TEST(SpawnCatalogsTest, MonsterEntryForUnknownTypeReturnsNull) {
    EXPECT_EQ(monsterEntryFor("no-existe"), nullptr);
}

TEST(SpawnCatalogsTest, CitizenEntryForKnownTypeReturnsIt) {
    const CitizenCatalogEntry* entry = citizenEntryFor("priest");
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->type, "priest");
}

TEST(SpawnCatalogsTest, CitizenEntryForUnknownTypeReturnsNull) {
    EXPECT_EQ(citizenEntryFor("no-existe"), nullptr);
}
