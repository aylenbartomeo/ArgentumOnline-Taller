#include <gtest/gtest.h>

#include "../client/src/ui/GroundItemLabel.h"

TEST(GroundItemLabelTest, AmountOneHasNoLabel) { EXPECT_FALSE(groundAmountLabel(1).has_value()); }

TEST(GroundItemLabelTest, AmountZeroHasNoLabel) { EXPECT_FALSE(groundAmountLabel(0).has_value()); }

TEST(GroundItemLabelTest, AmountManyShowsNumber) {
    auto label = groundAmountLabel(100);
    ASSERT_TRUE(label.has_value());
    EXPECT_EQ(*label, "100");
}
