#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "SpawnCatalogs.h"

TEST(SpawnCatalogsTest, MonsterCatalogHasSixEntries) {
    EXPECT_EQ(getMonsterCatalog().size(), 6u);
}

TEST(SpawnCatalogsTest, MonsterCatalogReferencesExpectedSheets) {
    const auto& catalog = getMonsterCatalog();
    auto find = [&](const std::string& type) {
        for (const auto& entry: catalog) {
            if (entry.type == type)
                return entry.sheet;
        }
        return std::string("");
    };
    EXPECT_EQ(find("goblin"), "1800.png");
    EXPECT_EQ(find("orc"), "1875.png");
    EXPECT_EQ(find("zombie"), "1892.png");
    EXPECT_EQ(find("spider"), "1052.png");
    EXPECT_EQ(find("golem"), "1140.png");
    EXPECT_EQ(find("skeleton"), "1238.png");
}

TEST(SpawnCatalogsTest, CitizenCatalogHasMerchantBankerPriest) {
    const auto& catalog = getCitizenCatalog();
    ASSERT_EQ(catalog.size(), 3u);
    std::vector<std::string> types;
    for (const auto& entry: catalog) types.push_back(entry.type);
    EXPECT_NE(std::find(types.begin(), types.end(), "merchant"), types.end());
    EXPECT_NE(std::find(types.begin(), types.end(), "banker"), types.end());
    EXPECT_NE(std::find(types.begin(), types.end(), "priest"), types.end());
}

TEST(SpawnCatalogsTest, ItemCatalogHasAtLeastOneEntry) {
    EXPECT_GE(getItemCatalog().size(), 1u);
}
