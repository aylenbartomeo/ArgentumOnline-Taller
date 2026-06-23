#include <gtest/gtest.h>

#include "CatalogSprites.h"
#include "OverlayRegistry.h"

TEST(CatalogSpritesTest, ItemOverlayIndicesAreNonZeroItemIds) {
    std::vector<int> idx = itemOverlayIndices();
    const std::vector<OverlayDef>& reg = getOverlayRegistry();
    ASSERT_FALSE(idx.empty());
    for (int i: idx) {
        EXPECT_NE(reg[i].itemId, 0);
    }
}
