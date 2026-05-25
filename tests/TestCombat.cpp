#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "model/FormulaEngine.h"
#include "model/combat/CombatManager.h"
#include "model/entities/Monster.h"
#include "model/entities/Player.h"
#include "model/items/Weapon.h"

// MockAttackable se usa como TARGET (no como attacker)
class MockAttackable: public Attackable {
public:
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(bool, isDead, (), (const, override));
    MOCK_METHOD(bool, canBeAttacked, (), (const, override));
    MOCK_METHOD(Position, getPosition, (), (const, override));
    MOCK_METHOD(uint16_t, getStrength, (), (const, override));
    MOCK_METHOD(uint16_t, getIntelligence, (), (const, override));
    MOCK_METHOD(uint16_t, getAgility, (), (const, override));
    MOCK_METHOD(uint16_t, getLevel, (), (const, override));
    MOCK_METHOD(uint16_t, getMaxHp, (), (const, override));
    MOCK_METHOD(int, getDefense, (), (const, override));
    MOCK_METHOD(void, receiveDamage, (int), (override));
    MOCK_METHOD(void, handleDeath, (), (override));
    MOCK_METHOD(bool, canEngageInCombatWith, (const Attackable&), (const, override));
};

// Helper: crea un Player base para tests
static Player makeTestPlayer(uint32_t id = 1) {
    std::string name = "TestPlayer";
    RaceConfig race = {1.0f, 1.0f, 1.0f};
    CharacterClassConfig cls = {1.0f, 1.0f, 1.0f, false};
    PlayerConfig cfg = {15, 15, 15, 15, 1, 0, 0};
    return Player(id, id, name, race, cls, cfg, FormulaEngine::getInstance());
}

// Helper: crea un Monster base para tests
static Monster makeTestMonster(uint32_t id = 10000, Position pos = {0, 0}) {
    MonsterConfig cfg = {100, 10, 0, 5, 15, 5, 2, 1, "zone"};
    return Monster(id, NPCType::GOBLIN, pos, cfg);
}

// --- Player Attack Tests ---

TEST(CombatManagerTest, PlayerAttackNoWeapon) {
    Player attacker = makeTestPlayer();
    MockAttackable target;

    // Sin arma equipada, no debería llamar a receiveDamage
    EXPECT_CALL(target, receiveDamage(testing::_)).Times(0);

    CombatManager::getInstance().processAttack(attacker, target);
}

TEST(CombatManagerTest, PlayerAttackOutOfRange) {
    Player attacker = makeTestPlayer();
    MockAttackable target;

    // Equipar un arma con rango 1
    Weapon sword(1, "TestSword", 10, 20, WeaponType::MELEE, 1, 0);
    attacker.getEquipment().equipWeapon(&sword);

    // Target a distancia 5, rango 1
    EXPECT_CALL(target, getPosition()).WillRepeatedly(testing::Return(Position{5, 0}));

    EXPECT_CALL(target, receiveDamage(testing::_)).Times(0);

    CombatManager::getInstance().processAttack(attacker, target);
}

TEST(CombatManagerTest, PlayerPhysicalAttackSuccessful) {
    Player attacker = makeTestPlayer();
    MockAttackable target;

    Weapon sword(1, "TestSword", 10, 20, WeaponType::MELEE, 1, 0);
    attacker.getEquipment().equipWeapon(&sword);

    EXPECT_CALL(target, getPosition()).WillRepeatedly(testing::Return(Position{0, 0}));
    EXPECT_CALL(target, isDead()).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(target, canBeAttacked()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(target, getAgility()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(target, getDefense()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(target, getLevel()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(target, getName()).WillRepeatedly(testing::Return("Target"));

    EXPECT_CALL(target, receiveDamage(testing::_)).Times(1);

    CombatManager::getInstance().processAttack(attacker, target);
}

TEST(CombatManagerTest, PlayerMagicAttackInsufficientMana) {
    Player attacker = makeTestPlayer();
    MockAttackable target;

    // Arma mágica con costo de maná 999 (más de lo que tiene el Player)
    Weapon staff(2, "TestStaff", 10, 20, WeaponType::MAGIC, 5, 999);
    attacker.getEquipment().equipWeapon(&staff);

    // No debería recibir daño porque no hay maná suficiente
    EXPECT_CALL(target, receiveDamage(testing::_)).Times(0);

    CombatManager::getInstance().processAttack(attacker, target);
}

TEST(CombatManagerTest, PlayerMagicAttackSuccessful) {
    Player attacker = makeTestPlayer();
    MockAttackable target;

    // Arma mágica con costo de maná 1 (el Player tiene 15)
    Weapon staff(2, "TestStaff", 10, 20, WeaponType::MAGIC, 5, 1);
    attacker.getEquipment().equipWeapon(&staff);

    EXPECT_CALL(target, getPosition()).WillRepeatedly(testing::Return(Position{0, 0}));
    EXPECT_CALL(target, isDead()).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(target, canBeAttacked()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(target, getAgility()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(target, getDefense()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(target, getLevel()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(target, getName()).WillRepeatedly(testing::Return("Target"));

    EXPECT_CALL(target, receiveDamage(testing::_)).Times(1);

    CombatManager::getInstance().processAttack(attacker, target);
}

// --- Monster Attack Tests ---

TEST(CombatManagerTest, MonsterAttackOutOfRange) {
    Monster attacker = makeTestMonster(10000, {0, 0});
    MockAttackable target;

    // Monster con attack_range 2, target a distancia 5
    EXPECT_CALL(target, getPosition()).WillRepeatedly(testing::Return(Position{5, 0}));

    EXPECT_CALL(target, receiveDamage(testing::_)).Times(0);

    CombatManager::getInstance().processAttack(attacker, target);
}

TEST(CombatManagerTest, MonsterAttackSuccessful) {
    Monster attacker = makeTestMonster(10000, {0, 0});
    MockAttackable target;

    // Monster con attack_range 2, target a distancia 1
    EXPECT_CALL(target, getPosition()).WillRepeatedly(testing::Return(Position{1, 0}));
    EXPECT_CALL(target, isDead()).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(target, canBeAttacked()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(target, getAgility()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(target, getDefense()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(target, getName()).WillRepeatedly(testing::Return("Target"));

    EXPECT_CALL(target, receiveDamage(testing::_)).Times(1);

    CombatManager::getInstance().processAttack(attacker, target);
}
