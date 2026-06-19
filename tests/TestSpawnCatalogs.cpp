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
    EXPECT_EQ(find("goblin"), "monsters/goblin1.png");
    EXPECT_EQ(find("orc"), "monsters/orco1.png");
    EXPECT_EQ(find("zombie"), "monsters/zombie1.png");
    EXPECT_EQ(find("spider"), "monsters/spider1.png");
    EXPECT_EQ(find("golem"), "monsters/golem1.png");
    EXPECT_EQ(find("skeleton"), "monsters/skeleton1.png");
}

TEST(SpawnCatalogsTest, CitizenCatalogHasMerchantBankerPriest) {
    const auto& catalog = getCitizenCatalog();
    ASSERT_EQ(catalog.size(), 3u);
    auto find = [&](const std::string& type) {
        auto it = std::find_if(catalog.begin(), catalog.end(),
                               [&type](const auto& entry) { return entry.type == type; });
        return it != catalog.end() ? it->sheet : std::string("");
    };
    EXPECT_EQ(find("merchant"), "1077.png");
    EXPECT_EQ(find("banker"), "1071.png");
    EXPECT_EQ(find("priest"), "1910.png");
}
