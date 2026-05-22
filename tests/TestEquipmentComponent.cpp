#include <gtest/gtest.h>
#include "model/components/EquipmentComponent.h"
#include "model/items/BodyArmor.h"

TEST(EquipmentComponentTest, EquipmentCalculatesDefense) {
    EquipmentComponent eq;
    BodyArmor armor(1001, "Armadura de cuero", 5,100); // ID, Name, min and max armor

    
    eq.equipBodyArmor(&armor);
    EXPECT_EQ(eq.getDefense(), 5);
}