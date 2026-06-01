#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "SpawnCatalogs.h"

TEST(SpawnCatalogsTest, MonsterCatalogHasSixEntries) { EXPECT_EQ(getMonsterCatalog().size(), 6u); }

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
    auto find = [&](const std::string& type) {
        for (const auto& entry: catalog) {
            if (entry.type == type)
                return entry.sheet;
        }
        return std::string("");
    };
    EXPECT_EQ(find("merchant"), "1077.png");
    EXPECT_EQ(find("banker"), "1071.png");
    EXPECT_EQ(find("priest"), "1910.png");
}

TEST(SpawnCatalogsTest, ItemCatalogHasAtLeastOneEntry) { EXPECT_GE(getItemCatalog().size(), 1u); }
