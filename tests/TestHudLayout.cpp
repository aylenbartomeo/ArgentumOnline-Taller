#include <gtest/gtest.h>

#include "../client/src/ui/HudLayout.h"

TEST(HudLayoutTest, BarFillProporcional) {
    EXPECT_EQ(barFillWidth(50, 100, 200), 100);
    EXPECT_EQ(barFillWidth(100, 100, 200), 200);
    EXPECT_EQ(barFillWidth(0, 100, 200), 0);
}

TEST(HudLayoutTest, BarFillClampYDivCero) {
    EXPECT_EQ(barFillWidth(150, 100, 200), 200);
    EXPECT_EQ(barFillWidth(10, 0, 200), 0);
    EXPECT_EQ(barFillWidth(-5, 100, 200), 0);
}

TEST(HudLayoutTest, SlotRectGrilla) {
    SlotRect a = inventorySlotRect(0, 4, 32, 2, 100, 200);
    EXPECT_EQ(a.x, 100);
    EXPECT_EQ(a.y, 200);
    EXPECT_EQ(a.w, 32);
    EXPECT_EQ(a.h, 32);

    SlotRect b = inventorySlotRect(1, 4, 32, 2, 100, 200);
    EXPECT_EQ(b.x, 134);
    EXPECT_EQ(b.y, 200);

    SlotRect c = inventorySlotRect(4, 4, 32, 2, 100, 200);
    EXPECT_EQ(c.x, 100);
    EXPECT_EQ(c.y, 234);
}
