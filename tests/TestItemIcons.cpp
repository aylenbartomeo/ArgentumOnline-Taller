#include <string>

#include <gtest/gtest.h>

#include "../client/src/ui/ItemIcons.h"

TEST(ItemIconsTest, EquipSlotPorRango) {
    EXPECT_EQ(equipSlotForItem(1005), EquipSlot::ARMOR);
    EXPECT_EQ(equipSlotForItem(1015), EquipSlot::HELMET);
    EXPECT_EQ(equipSlotForItem(1025), EquipSlot::SHIELD);
    EXPECT_EQ(equipSlotForItem(2007), EquipSlot::WEAPON);
    EXPECT_EQ(equipSlotForItem(3001), EquipSlot::NONE);
    EXPECT_EQ(equipSlotForItem(1), EquipSlot::NONE);
}

TEST(ItemIconsTest, IconoPorCategoria) {
    EXPECT_EQ(std::string(iconForItem(2007)), "resources/items/icon_weapon.png");
    EXPECT_EQ(std::string(iconForItem(1005)), "resources/items/icon_armor.png");
    EXPECT_EQ(std::string(iconForItem(1015)), "resources/items/icon_helmet.png");
    EXPECT_EQ(std::string(iconForItem(1025)), "resources/items/icon_shield.png");
    EXPECT_EQ(std::string(iconForItem(3001)), "resources/items/icon_potion.png");
    EXPECT_EQ(std::string(iconForItem(99999)), "resources/items/icon_unknown.png");
}
