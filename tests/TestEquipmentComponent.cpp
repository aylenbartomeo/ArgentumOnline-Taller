#include <gtest/gtest.h>
#include "model/components/EquipmentComponent.h"
#include "model/items/BodyArmor.h"
#include "model/items/Helmet.h"
#include "model/items/Shield.h"
#include "model/items/Weapon.h"

// Centraliza la creación de ítems para no repetirla en cada test.
// Todos los ítems tienen min == max para que getDefense()/calculateDamage() sean deterministas y los EXPECT sean exactos.
class EquipmentComponentTest : public ::testing::Test {
protected:
    // Armaduras (min == max → rollDefense() siempre devuelve el mismo valor)
    BodyArmor armor   {1001, "Armadura de cuero",  5,  5};
    BodyArmor armor2  {1002, "Armadura de placas", 20, 20};
    Helmet    helmet  {2001, "Casco de hierro",     6,  6};
    Shield    shield  {3001, "Escudo de tortuga",   2,  2};

    // Armas
    Weapon sword  {4001, "Espada",     3,  3, WeaponType::MELEE,   1};
    Weapon bow    {4002, "Arco simple",2,  2, WeaponType::RANGED,  5};
    Weapon staff  {4003, "Vara de fresno", 2, 2, WeaponType::MAGIC, 3, 5};

    EquipmentComponent eq;
};

// ============================================================================
// 1. ESTADO INICIAL
// ============================================================================
TEST_F(EquipmentComponentTest, StartsWithNoEquipment) {
    EXPECT_EQ(eq.getBodyArmor(), nullptr);
    EXPECT_EQ(eq.getHelmet(),    nullptr);
    EXPECT_EQ(eq.getShield(),    nullptr);
    EXPECT_EQ(eq.getWeapon(),    nullptr);
}

TEST_F(EquipmentComponentTest, DefenseIsZeroWithNothingEquipped) {
    EXPECT_EQ(eq.getDefense(), 0);
}

// ============================================================================
// 2. EQUIPAR Y LEER DEFENSA POR SLOT
// ============================================================================
TEST_F(EquipmentComponentTest, EquipBodyArmorAndCheckDefense) {
    eq.equipBodyArmor(&armor);
    EXPECT_EQ(eq.getBodyArmor(), &armor);
    EXPECT_EQ(eq.getDefense(),   5);
}

TEST_F(EquipmentComponentTest, EquipHelmetAndCheckDefense) {
    eq.equipHelmet(&helmet);
    EXPECT_EQ(eq.getHelmet(),  &helmet);
    EXPECT_EQ(eq.getDefense(), 6);
}

TEST_F(EquipmentComponentTest, EquipShieldAndCheckDefense) {
    eq.equipShield(&shield);
    EXPECT_EQ(eq.getShield(),  &shield);
    EXPECT_EQ(eq.getDefense(), 2);
}

TEST_F(EquipmentComponentTest, EquipWeaponAndCheckPointer) {
    eq.equipWeapon(&sword);
    EXPECT_EQ(eq.getWeapon(), &sword);
}

// ============================================================================
// 3. DEFENSA ACUMULADA (varios slots a la vez)
// ============================================================================
TEST_F(EquipmentComponentTest, TotalDefenseIsTheSumOfAllSlots) {
    eq.equipBodyArmor(&armor);   // +5
    eq.equipHelmet(&helmet);     // +6
    eq.equipShield(&shield);     // +2
    // Total esperado: 13
    EXPECT_EQ(eq.getDefense(), 13);
}

TEST_F(EquipmentComponentTest, DefenseWithOnlyArmorAndHelmet) {
    eq.equipBodyArmor(&armor);   // +5
    eq.equipHelmet(&helmet);     // +6
    EXPECT_EQ(eq.getDefense(), 11);
}

// ============================================================================
// 4. REEMPLAZO DE ÍTEMS (devuelve el ID del anterior)
// ============================================================================
TEST_F(EquipmentComponentTest, ReplacingArmorReturnsOldId) {
    eq.equipBodyArmor(&armor);
    uint32_t replaced = eq.equipBodyArmor(&armor2);

    EXPECT_EQ(replaced,          static_cast<uint32_t>(armor.getId()));
    EXPECT_EQ(eq.getBodyArmor(), &armor2);
    EXPECT_EQ(eq.getDefense(),   20);  // ahora tiene la nueva armadura
}

TEST_F(EquipmentComponentTest, ReplacingWeaponReturnsOldId) {
    eq.equipWeapon(&sword);
    uint32_t replaced = eq.equipWeapon(&bow);

    EXPECT_EQ(replaced,        static_cast<uint32_t>(sword.getId()));
    EXPECT_EQ(eq.getWeapon(),  &bow);
}

TEST_F(EquipmentComponentTest, EquipFirstArmorReturnsZero) {
    // Si no había nada equipado, el ID del reemplazado debe ser 0
    uint32_t replaced = eq.equipBodyArmor(&armor);
    EXPECT_EQ(replaced, 0u);
}

// ============================================================================
// 5. DESEQUIPAR EXPLÍCITAMENTE
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
//     // Desequipar algo que nunca fue equipado no debe crashear ni devolver id válido
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
// 6. equipItem (despacho polimórfico por tipo de ítem)
// ============================================================================
TEST_F(EquipmentComponentTest, EquipItemDispatchesToCorrectSlot) {
    // BodyArmor se despacha al slot de armadura
    eq.equipItem(&armor);
    EXPECT_EQ(eq.getBodyArmor(), &armor);

    // Helmet al slot de casco
    eq.equipItem(&helmet);
    EXPECT_EQ(eq.getHelmet(), &helmet);

    // Shield al slot de escudo
    eq.equipItem(&shield);
    EXPECT_EQ(eq.getShield(), &shield);

    // Weapon al slot de arma
    eq.equipItem(&sword);
    EXPECT_EQ(eq.getWeapon(), &sword);
}

TEST_F(EquipmentComponentTest, EquipItemWithNullptrDoesNothing) {
    eq.equipItem(nullptr);
    EXPECT_EQ(eq.getDefense(), 0);
    EXPECT_EQ(eq.getWeapon(),  nullptr);
}

// ============================================================================
// 7. ARMAS MÁGICAS (báculos con coste de maná)
// ============================================================================
TEST_F(EquipmentComponentTest, MagicWeaponEquipsCorrectly) {
    eq.equipWeapon(&staff);
    EXPECT_EQ(eq.getWeapon(),              &staff);
    EXPECT_EQ(eq.getWeapon()->getManaCost(), 5);
    EXPECT_EQ(eq.getWeapon()->getType(),   WeaponType::MAGIC);
}

// ============================================================================
// 8. getEquippedWeapon (alias mutable)
// ============================================================================
TEST_F(EquipmentComponentTest, GetEquippedWeaponReturnsNullWhenEmpty) {
    EXPECT_EQ(eq.getEquippedWeapon(), nullptr);
}

TEST_F(EquipmentComponentTest, GetEquippedWeaponMatchesGetWeapon) {
    eq.equipWeapon(&bow);
    EXPECT_EQ(eq.getEquippedWeapon(), eq.getWeapon());
}