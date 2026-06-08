#include <gtest/gtest.h>

#include "model/components/EquipmentComponent.h"
#include "model/items/BodyArmor.h"
#include "model/items/Helmet.h"
#include "model/items/Shield.h"
#include "model/items/Weapon.h"

// Centraliza la creaciÃ³n de Ã­tems para no repetirla en cada test.
// Todos los Ã­tems tienen min == max para que getDefense()/calculateDamage() sean deterministas y
// los EXPECT sean exactos.
class EquipmentComponentTest: public ::testing::Test {
protected:
    // Armaduras (min == max â†’ rollDefense() siempre devuelve el mismo valor)
    BodyArmor armor{1001, "Armadura de cuero", 5, 5};
    BodyArmor armor2{1002, "Armadura de placas", 20, 20};
    Helmet helmet{2001, "Casco de hierro", 6, 6};
    Shield shield{3001, "Escudo de tortuga", 2, 2};

    // Armas
    Weapon sword{4001, "Espada", 100, WeaponType::MELEE, 3, 3, 1};
    Weapon bow{4002, "Arco simple", 80, WeaponType::RANGED, 2, 2, 5};
    Weapon staff{4003, "Vara de fresno", 120, WeaponType::MAGIC, 2, 2, 3, 5};

    EquipmentComponent eq;
};

// ============================================================================
// 1. ESTADO INICIAL
// ============================================================================
TEST_F(EquipmentComponentTest, StartsWithNoEquipment) {
    EXPECT_EQ(eq.getBodyArmor(), nullptr);
    EXPECT_EQ(eq.getHelmet(), nullptr);
    EXPECT_EQ(eq.getShield(), nullptr);
    EXPECT_EQ(eq.getWeapon(), nullptr);
}

TEST_F(EquipmentComponentTest, DefenseIsZeroWithNothingEquipped) {
    EXPECT_EQ(eq.calculateCurrentDefense(), 0);
}

// ============================================================================
// 2. EQUIPAR Y LEER DEFENSA POR SLOT
// ============================================================================
TEST_F(EquipmentComponentTest, EquipBodyArmorAndCheckDefense) {
    eq.equipBodyArmor(&armor, 0);
    EXPECT_EQ(eq.getBodyArmor(), &armor);
    EXPECT_EQ(eq.calculateCurrentDefense(), 5);
}

TEST_F(EquipmentComponentTest, EquipHelmetAndCheckDefense) {
    eq.equipHelmet(&helmet, 1);
    EXPECT_EQ(eq.getHelmet(), &helmet);
    EXPECT_EQ(eq.calculateCurrentDefense(), 6);
}

TEST_F(EquipmentComponentTest, EquipShieldAndCheckDefense) {
    eq.equipShield(&shield, 2);
    EXPECT_EQ(eq.getShield(), &shield);
    EXPECT_EQ(eq.calculateCurrentDefense(), 2);
}

TEST_F(EquipmentComponentTest, EquipWeaponAndCheckPointer) {
    eq.equipWeapon(&sword, 3);
    EXPECT_EQ(eq.getWeapon(), &sword);
}

// ============================================================================
// 3. DEFENSA ACUMULADA (varios slots a la vez)
// ============================================================================
TEST_F(EquipmentComponentTest, TotalDefenseIsTheSumOfAllSlots) {
    eq.equipBodyArmor(&armor, 0);  // +5
    eq.equipHelmet(&helmet, 1);    // +6
    eq.equipShield(&shield, 2);    // +2
    // Total esperado: 13
    EXPECT_EQ(eq.calculateCurrentDefense(), 13);
}

TEST_F(EquipmentComponentTest, DefenseWithOnlyArmorAndHelmet) {
    eq.equipBodyArmor(&armor, 0);  // +5
    eq.equipHelmet(&helmet, 1);    // +6
    EXPECT_EQ(eq.calculateCurrentDefense(), 11);
}

// Tests de reemplazo eliminados porque el comportamiento cambió

// ============================================================================
// 5. DESEQUIPAR EXPLÃCITAMENTE
// ============================================================================
// TEST_F(EquipmentComponentTest, UnequipBodyArmorRestoresDefenseToZero) {
//     eq.equipBodyArmor(&armor);
//     ASSERT_EQ(eq.getDefense(), 5);

//     uint32_t unequipped_id = eq.unequip_body_armor();
//     EXPECT_EQ(unequipped_id,     static_cast<uint32_t>(armor.getId()));
//     EXPECT_EQ(eq.getBodyArmor(), nullptr);
//     EXPECT_EQ(eq.getDefense(),   0);
// }

// TEST_F(EquipmentComponentTest, UnequipHelmetRestoresDefense) {
//     eq.equipBodyArmor(&armor);
//     eq.equipHelmet(&helmet);
//     ASSERT_EQ(eq.getDefense(), 11);

//     eq.unequip_helmet();
//     EXPECT_EQ(eq.getHelmet(),  nullptr);
//     EXPECT_EQ(eq.getDefense(), 5);   // solo queda la armadura
// }

// TEST_F(EquipmentComponentTest, UnequipFromEmptySlotReturnsZero) {
//     // Desequipar algo que nunca fue equipado no debe crashear ni devolver id vÃ¡lido
//     uint32_t id = eq.unequip_helmet();
//     EXPECT_EQ(id, 0u);
// }

// TEST_F(EquipmentComponentTest, UnequipWeaponLeavesNoWeaponEquipped) {
//     eq.equipWeapon(&sword);
//     eq.unequip_weapon();
//     EXPECT_EQ(eq.getWeapon(),         nullptr);
//     EXPECT_EQ(eq.getEquippedWeapon(), nullptr);
// }

// ============================================================================
// 6. equipItem (despacho polimÃ³rfico por tipo de Ã­tem)
// ============================================================================
TEST_F(EquipmentComponentTest, EquipItemDispatchesToCorrectSlot) {
    // BodyArmor se despacha al slot de armadura
    eq.equipItem(&armor, 0);
    EXPECT_EQ(eq.getBodyArmor(), &armor);

    // Helmet al slot de casco
    eq.equipItem(&helmet, 1);
    EXPECT_EQ(eq.getHelmet(), &helmet);

    // Shield al slot de escudo
    eq.equipItem(&shield, 2);
    EXPECT_EQ(eq.getShield(), &shield);

    // Weapon al slot de arma
    eq.equipItem(&sword, 3);
    EXPECT_EQ(eq.getWeapon(), &sword);
}

TEST_F(EquipmentComponentTest, EquipItemWithNullptrDoesNothing) {
    eq.equipItem(nullptr, 0);
    EXPECT_EQ(eq.calculateCurrentDefense(), 0);
    EXPECT_EQ(eq.getWeapon(), nullptr);
}

// ============================================================================
// 7. ARMAS MÃ GICAS (bÃ¡culos con coste de manÃ¡)
// ============================================================================
TEST_F(EquipmentComponentTest, MagicWeaponEquipsCorrectly) {
    eq.equipWeapon(&staff, 0);
    EXPECT_EQ(eq.getWeapon(), &staff);
    EXPECT_EQ(eq.getWeapon()->getManaCost(), 5);
    EXPECT_EQ(eq.getWeapon()->getType(), WeaponType::MAGIC);
}

// ============================================================================
// 8. getEquippedWeapon (alias mutable)
// ============================================================================
TEST_F(EquipmentComponentTest, GetEquippedWeaponReturnsNullWhenEmpty) {
    EXPECT_EQ(eq.getEquippedWeapon(), nullptr);
}

TEST_F(EquipmentComponentTest, GetEquippedWeaponMatchesGetWeapon) {
    eq.equipWeapon(&bow, 0);
    EXPECT_EQ(eq.getEquippedWeapon(), eq.getWeapon());
}
