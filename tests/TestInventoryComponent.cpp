#include <gtest/gtest.h>

#include "model/components/InventoryComponent.h"

// Inventario estándar: 5 slots, 5000 oro seguro, 10000 oro máximo.
class InventoryComponentTest: public ::testing::Test {
protected:
    static constexpr int SLOTS = 5;
    static constexpr int SAFE_GOLD = 5000;
    static constexpr int MAX_GOLD = 10000;

    InventoryConfig cfg{SLOTS, MAX_GOLD};
    InventoryComponent inv{cfg, SAFE_GOLD};
};

// ============================================================================
// 1. ESTADO INICIAL
// ============================================================================
TEST_F(InventoryComponentTest, StartsWithZeroGold) { EXPECT_EQ(inv.getGold(), 0u); }

TEST_F(InventoryComponentTest, StartsWithCorrectSlotCount) { EXPECT_EQ(inv.getSize(), SLOTS); }

TEST_F(InventoryComponentTest, AllSlotsStartEmpty) {
    for (uint8_t i = 0; i < inv.getSize(); ++i) {
        EXPECT_FALSE(inv.inspectSlot(i).has_value())
                << "Slot " << static_cast<int>(i) << " deberia estar vacio";
    }
}

// ============================================================================
// 2. ORO — AGREGAR
// ============================================================================
TEST_F(InventoryComponentTest, AddGoldBasicCase) {
    EXPECT_TRUE(inv.addGold(1000));
    EXPECT_EQ(inv.getGold(), 1000u);
}

TEST_F(InventoryComponentTest, AddGoldRespectsMaximum) {
    inv.addGold(MAX_GOLD + 5000);  // intenta pasarse del tope
    EXPECT_EQ(inv.getGold(), static_cast<uint32_t>(MAX_GOLD));
}

TEST_F(InventoryComponentTest, AddGoldWhenAlreadyFullReturnsFalse) {
    inv.addGold(MAX_GOLD);
    EXPECT_FALSE(inv.addGold(1));  // ya está al tope
    EXPECT_EQ(inv.getGold(), static_cast<uint32_t>(MAX_GOLD));
}

TEST_F(InventoryComponentTest, AddZeroGoldReturnsFalse) {
    EXPECT_FALSE(inv.addGold(0));
    EXPECT_EQ(inv.getGold(), 0u);
}

TEST_F(InventoryComponentTest, AddGoldAccumulatesCorrectly) {
    inv.addGold(3000);
    inv.addGold(2000);
    EXPECT_EQ(inv.getGold(), 5000u);
}

// ============================================================================
// 3. ORO — REMOVER
// ============================================================================
TEST_F(InventoryComponentTest, RemoveGoldBasicCase) {
    inv.addGold(5000);
    EXPECT_TRUE(inv.removeGold(2000));
    EXPECT_EQ(inv.getGold(), 3000u);
}

TEST_F(InventoryComponentTest, RemoveMoreGoldThanAvailableReturnsFalse) {
    inv.addGold(500);
    EXPECT_FALSE(inv.removeGold(1000));
    EXPECT_EQ(inv.getGold(), 500u);  // el saldo no cambia
}

TEST_F(InventoryComponentTest, RemoveExactGoldLeavesZero) {
    inv.addGold(3000);
    EXPECT_TRUE(inv.removeGold(3000));
    EXPECT_EQ(inv.getGold(), 0u);
}

// ============================================================================
// 4. ORO — MUERTE Y EXCESO (dropExcessGold)
// ============================================================================
TEST_F(InventoryComponentTest, DropExcessGoldReturnsZeroWhenUnderSafeLimit) {
    inv.addGold(4000);  // 4000 < 5000 (safe limit)
    EXPECT_EQ(inv.dropExcessGold(), 0u);
    EXPECT_EQ(inv.getGold(), 4000u);
}

TEST_F(InventoryComponentTest, DropExcessGoldReturnsZeroAtExactSafeLimit) {
    inv.addGold(SAFE_GOLD);
    EXPECT_EQ(inv.dropExcessGold(), 0u);
    EXPECT_EQ(inv.getGold(), static_cast<uint32_t>(SAFE_GOLD));
}

TEST_F(InventoryComponentTest, DropExcessGoldDropsCorrectAmount) {
    inv.addGold(7000);  // 7000 - 5000 (safe) = 2000 de exceso
    uint32_t dropped = inv.dropExcessGold();

    EXPECT_EQ(dropped, 2000u);
    EXPECT_EQ(inv.getGold(), static_cast<uint32_t>(SAFE_GOLD));
}

TEST_F(InventoryComponentTest, SafeLimitUpdateAffectsDropExcess) {
    inv.addGold(8000);
    inv.updateSafeLimit(6000);  // subimos el techo seguro (nivel up)
    uint32_t dropped = inv.dropExcessGold();

    EXPECT_EQ(dropped, 2000u);  // 8000 - 6000
    EXPECT_EQ(inv.getGold(), 6000u);
}

// ============================================================================
// 5. ÍTEMS — AGREGAR
// ============================================================================
TEST_F(InventoryComponentTest, AddItemFillsSlot) {
    EXPECT_EQ(inv.addItem(101, 5), 0);
    auto slot = inv.inspectSlot(0);
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot->item_id, 101u);
    EXPECT_EQ(slot->amount, 5);
}

TEST_F(InventoryComponentTest, AddZeroAmountReturnsFalse) { EXPECT_EQ(inv.addItem(101, 0), 0); }

TEST_F(InventoryComponentTest, AddItemIdZeroReturnsFalse) { EXPECT_EQ(inv.addItem(0, 5), 5); }

TEST_F(InventoryComponentTest, StackingSameItemInSameSlot) {
    inv.addItem(101, 10);
    inv.addItem(101, 20);  // debe acumularse en el mismo slot
    auto slot = inv.inspectSlot(0);
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot->amount, 30);
}

TEST_F(InventoryComponentTest, DifferentItemsUseSeparateSlots) {
    inv.addItem(101, 1);
    inv.addItem(202, 1);

    auto slot0 = inv.inspectSlot(0);
    auto slot1 = inv.inspectSlot(1);
    ASSERT_TRUE(slot0.has_value());
    ASSERT_TRUE(slot1.has_value());
    EXPECT_NE(slot0->item_id, slot1->item_id);
}

TEST_F(InventoryComponentTest, AddItemFailsWhenAllSlotsFull) {
    // Llenamos los 5 slots con ítems distintos
    for (uint32_t i = 1; i <= SLOTS; ++i) {
        inv.addItem(i * 100, 1);
    }
    // El sexto ítem no debería entrar
    EXPECT_GT(inv.addItem(999, 1), 0);
}

// ============================================================================
// 6. ÍTEMS — REMOVER
// ============================================================================
TEST_F(InventoryComponentTest, RemoveItemPartialAmount) {
    inv.addItem(101, 10);
    uint16_t removed = inv.removeItem(0, 4);

    EXPECT_EQ(removed, 4);
    auto slot = inv.inspectSlot(0);
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot->amount, 6);
}

TEST_F(InventoryComponentTest, RemoveAllItemsClearsSlot) {
    inv.addItem(101, 5);
    uint16_t removed = inv.removeItem(0, 5);

    EXPECT_EQ(removed, 5);
    EXPECT_FALSE(inv.inspectSlot(0).has_value());  // slot vacío
}

TEST_F(InventoryComponentTest, RemoveMoreThanAvailableOnlyRemovesExisting) {
    inv.addItem(101, 3);
    uint16_t removed = inv.removeItem(0, 100);

    EXPECT_EQ(removed, 3);
    EXPECT_FALSE(inv.inspectSlot(0).has_value());
}

TEST_F(InventoryComponentTest, RemoveFromEmptySlotReturnsZero) {
    uint16_t removed = inv.removeItem(0, 5);
    EXPECT_EQ(removed, 0);
}

TEST_F(InventoryComponentTest, RemoveFromOutOfBoundsSlotReturnsZero) {
    uint16_t removed = inv.removeItem(255, 1);
    EXPECT_EQ(removed, 0);
}

// ============================================================================
// 7. INSPECCIÓN DE SLOTS
// ============================================================================
TEST_F(InventoryComponentTest, InspectSlotReturnsNulloptOnEmptySlot) {
    EXPECT_FALSE(inv.inspectSlot(0).has_value());
}

TEST_F(InventoryComponentTest, InspectSlotReturnsCorrectData) {
    inv.addItem(777, 42);
    auto slot = inv.inspectSlot(0);
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot->item_id, 777u);
    EXPECT_EQ(slot->amount, 42);
}

TEST_F(InventoryComponentTest, InspectOutOfBoundsSlotReturnsNullopt) {
    EXPECT_FALSE(inv.inspectSlot(200).has_value());
}

// ============================================================================
// 8. SLOT REUTILIZABLE TRAS VACIARSE
// ============================================================================
TEST_F(InventoryComponentTest, SlotIsReusableAfterBeingCleared) {
    inv.addItem(101, 5);
    inv.removeItem(0, 5);  // vaciamos el slot 0

    // Ahora debería poder usarse para otro ítem
    EXPECT_EQ(inv.addItem(202, 3), 0);
    auto slot = inv.inspectSlot(0);
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot->item_id, 202u);
}
