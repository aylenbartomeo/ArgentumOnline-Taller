#include <gtest/gtest.h>

#include "model/components/EquipmentComponent.h"
#include "model/interfaces/CombatStrategies.h"
#include "model/items/BodyArmor.h"
#include "model/items/Helmet.h"
#include "model/items/Shield.h"
#include "model/items/Weapon.h"
#include "model/items/WeaponFactory.h"

class EquipmentComponentTest: public ::testing::Test {
protected:
    BodyArmor armor{1001, "Armadura de cuero", 0, 5, 5};
    BodyArmor armor2{1002, "Armadura de placas", 0, 20, 20};
    Helmet helmet{2001, "Casco de hierro", 0, 6, 6};
    Shield shield{3001, "Escudo de tortuga", 0, 2, 2};

    // Armas
    Weapon sword{4001,
                 "Espada",
                 100,
                 WeaponType::MELEE,
                 3,
                 3,
                 1,
                 0,
                 WeaponFactory::createDeliveryStrategy(WeaponType::MELEE),
                 WeaponFactory::createHitEffectStrategy(WeaponType::MELEE)};
    Weapon bow{4002,
               "Arco simple",
               80,
               WeaponType::RANGED,
               2,
               2,
               5,
               0,
               WeaponFactory::createDeliveryStrategy(WeaponType::RANGED),
               WeaponFactory::createHitEffectStrategy(WeaponType::RANGED)};
    Weapon staff{4003,
                 "Vara de fresno",
                 120,
                 WeaponType::MAGIC,
                 2,
                 2,
                 3,
                 5,
                 WeaponFactory::createDeliveryStrategy(WeaponType::MAGIC),
                 WeaponFactory::createHitEffectStrategy(WeaponType::MAGIC)};

    EquipmentComponent eq;
};

// ============================================================================
// ESTADO INICIAL
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
// EQUIPAR Y LEER DEFENSA POR SLOT
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
// DEFENSA ACUMULADA
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

// ============================================================================
// EQUIPAR
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
// ARMAS MAGICAS
// ============================================================================
TEST_F(EquipmentComponentTest, MagicWeaponEquipsCorrectly) {
    eq.equipWeapon(&staff, 0);
    EXPECT_EQ(eq.getWeapon(), &staff);
    EXPECT_EQ(eq.getWeapon()->getManaCost(), 5);
    EXPECT_EQ(eq.getWeapon()->getType(), WeaponType::MAGIC);
}

// ============================================================================
// getEquippedWeapon
// ============================================================================
TEST_F(EquipmentComponentTest, GetEquippedWeaponReturnsNullWhenEmpty) {
    EXPECT_EQ(eq.getEquippedWeapon(), nullptr);
}

TEST_F(EquipmentComponentTest, GetEquippedWeaponMatchesGetWeapon) {
    eq.equipWeapon(&bow, 0);
    EXPECT_EQ(eq.getEquippedWeapon(), eq.getWeapon());
}
