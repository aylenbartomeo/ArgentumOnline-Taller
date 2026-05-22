#include <gtest/gtest.h>
#include "model/components/InventoryComponent.h"

TEST(InventoryComponentTest, InventoryRespectsConfigLimits) {
    InventoryConfig cfg = {20, 100000}; // Valores del inventory.toml
    InventoryComponent inv(cfg, 5000);
    
    // Intentar agregar oro mas alla del maximo configurado
    inv.addGold(200000); 
    EXPECT_EQ(inv.getGold(), 100000);
}