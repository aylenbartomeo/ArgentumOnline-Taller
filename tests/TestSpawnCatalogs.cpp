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
    EXPECT_EQ(find("goblin"), "4015.png");
    EXPECT_EQ(find("orc"), "4017.png");
    EXPECT_EQ(find("zombie"), "1892.png");
    EXPECT_EQ(find("spider"), "4151.png");
    EXPECT_EQ(find("golem"), "4091.png");
    EXPECT_EQ(find("skeleton"), "4079.png");
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
