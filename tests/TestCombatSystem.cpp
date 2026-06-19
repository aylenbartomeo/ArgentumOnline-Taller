// tests/TestCombatSystem.cpp
// Pruebas unitarias para el sistema de combate y sus estrategias de impacto

#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "model/clan/ClanRepository.h"
#include "model/entities/Monster.h"
#include "model/entities/Player.h"
#include "model/events/EventPublisher.h"
#include "model/interfaces/CombatStrategies.h"
#include "model/items/BodyArmor.h"
#include "model/items/Weapon.h"
#include "model/items/WeaponFactory.h"
#include "model/systems/BossSpawnSystem.h"
#include "model/systems/CombatSystem.h"
#include "model/systems/CombatTypes.h"

#include "TestHelpers.h"

// Callback de prueba para eventos de combate
class DummyCombatCallback: public ICombatEventCallback {
public:
    void onMonsterDeath(const Monster& monster, uint32_t killerDbId) override {}
    void onPlayerDeath(uint32_t dbId) override {}
};

// =========================================================================
// TEST 1: CombatSystem::applyDamageEffect Básico sin defensa
// =========================================================================
TEST(CombatSystemTest, ApplyDamageEffectBasicNoDefense) {
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;

    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, TestUtils::getTestServerConfig());

    Player attacker = TestUtils::makeTestPlayer(1);
    Player victim = TestUtils::makeTestPlayer(2);

    uint16_t initialHp = victim.getHp();

    // Daño min==max para que el roll de arma sea determinista (solo el esquive/crítico varía)
    AttackParams params{5, 5, 10, 0, false, 1.0f, 1.0f};

    CombatResult res = combatSystem.applyDamageEffect(attacker, victim, params);

    EXPECT_TRUE(res.attackHappened);
    uint16_t expectedHp =
            (res.damage >= initialHp) ? 0 : static_cast<uint16_t>(initialHp - res.damage);
    EXPECT_EQ(victim.getHp(), expectedHp);

    if (!res.evaded) {
        EXPECT_GT(res.damage, 0);
    }
}

// =========================================================================
// TEST 2: CombatSystem::applyDamageEffect Con defensa de armadura
// =========================================================================
TEST(CombatSystemTest, ApplyDamageEffectWithDefense) {
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, TestUtils::getTestServerConfig());

    Player attacker = TestUtils::makeTestPlayer(1);
    Player victimWithArmor = TestUtils::makeTestPlayer(2);
    Player victimNoArmor = TestUtils::makeTestPlayer(3);

    // Armadura con defensa fija min==max==10 para valores deterministas
    BodyArmor armor(1001, "Plates", 0, 10, 10);
    victimWithArmor.equipBodyArmor(&armor);

    AttackParams params{5, 5, 10, 0, false, 1.0f, 1.0f};

    CombatResult resWithArmor = combatSystem.applyDamageEffect(attacker, victimWithArmor, params);
    CombatResult resNoArmor = combatSystem.applyDamageEffect(attacker, victimNoArmor, params);

    EXPECT_TRUE(resWithArmor.attackHappened);
    EXPECT_TRUE(resNoArmor.attackHappened);

    uint16_t expectedHpWithArmor =
            (resWithArmor.damage >= victimWithArmor.getMaxHp()) ?
                    0 :
                    static_cast<uint16_t>(victimWithArmor.getMaxHp() - resWithArmor.damage);
    EXPECT_EQ(victimWithArmor.getHp(), expectedHpWithArmor);

    uint16_t expectedHpNoArmor =
            (resNoArmor.damage >= victimNoArmor.getMaxHp()) ?
                    0 :
                    static_cast<uint16_t>(victimNoArmor.getMaxHp() - resNoArmor.damage);
    EXPECT_EQ(victimNoArmor.getHp(), expectedHpNoArmor);

    if (!resWithArmor.evaded && !resNoArmor.evaded) {
        EXPECT_LE(resWithArmor.damage, resNoArmor.damage);
    }
}

// =========================================================================
// TEST 3: CombatSystem::applyHealEffect Básico
// =========================================================================
TEST(CombatSystemTest, ApplyHealEffectBasic) {
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, TestUtils::getTestServerConfig());

    Player victim = TestUtils::makeTestPlayer(1);
    victim.receiveDamage(10);
    ASSERT_LT(victim.getHp(), victim.getMaxHp());

    CombatResult res = combatSystem.applyHealEffect(victim);

    EXPECT_TRUE(res.attackHappened);
    EXPECT_EQ(victim.getHp(), victim.getMaxHp());
}

// =========================================================================
// TEST 4: MeleeDamageEffect Básico
// =========================================================================
TEST(CombatSystemTest, MeleeDamageEffectBasic) {
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, TestUtils::getTestServerConfig());

    Player attacker = TestUtils::makeTestPlayer(1);
    Player victim = TestUtils::makeTestPlayer(2);

    Weapon sword(4001, "Espada", 100, WeaponType::MELEE, 5, 5, 1, 0,
                 WeaponFactory::createDeliveryStrategy(WeaponType::MELEE),
                 WeaponFactory::createHitEffectStrategy(WeaponType::MELEE));

    uint16_t initialHp = victim.getHp();
    CombatModifiers modifiers{1.0f, 1.0f};

    MeleeDamageEffect effect;
    CombatResult res = effect.apply(attacker, victim, modifiers, sword, combatSystem);

    EXPECT_TRUE(res.attackHappened);
    uint16_t expectedHp =
            (res.damage >= initialHp) ? 0 : static_cast<uint16_t>(initialHp - res.damage);
    EXPECT_EQ(victim.getHp(), expectedHp);
    if (!res.evaded) {
        EXPECT_GT(res.damage, 0);
    }
}

// =========================================================================
// TEST 5: MagicDamageEffect Suficiente Maná
// =========================================================================
TEST(CombatSystemTest, MagicDamageEffectSufficientMana) {
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, TestUtils::getTestServerConfig());

    Player attacker = TestUtils::makeTestPlayer(1);
    Player victim = TestUtils::makeTestPlayer(2);

    attacker.restoreMana();
    uint16_t initialMana = attacker.getMana();
    uint16_t initialHp = victim.getHp();

    Weapon staff(4003, "Vara de fresno", 120, WeaponType::MAGIC, 5, 5, 3, 10,
                 WeaponFactory::createDeliveryStrategy(WeaponType::MAGIC),
                 WeaponFactory::createHitEffectStrategy(WeaponType::MAGIC));

    CombatModifiers modifiers{1.0f, 1.0f};
    MagicDamageEffect effect;
    CombatResult res = effect.apply(attacker, victim, modifiers, staff, combatSystem);

    EXPECT_TRUE(res.attackHappened);
    EXPECT_EQ(attacker.getMana(), initialMana);
    uint16_t expectedHp =
            (res.damage >= initialHp) ? 0 : static_cast<uint16_t>(initialHp - res.damage);
    EXPECT_EQ(victim.getHp(), expectedHp);
    if (!res.evaded) {
        EXPECT_GT(res.damage, 0);
    }
}

// =========================================================================
// TEST 7: MagicHealEffect Curación Exitosa
// =========================================================================
TEST(CombatSystemTest, MagicHealEffectSuccess) {
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, TestUtils::getTestServerConfig());

    Player attacker = TestUtils::makeTestPlayer(1);
    Player victim = TestUtils::makeTestPlayer(2);

    victim.receiveDamage(10);
    attacker.restoreMana();
    uint16_t initialMana = attacker.getMana();
    ASSERT_LT(victim.getHp(), victim.getMaxHp());

    Weapon flute(4004, "Flauta elfa", 120, WeaponType::MAGIC, 0, 0, 3, 5,
                 WeaponFactory::createDeliveryStrategy(WeaponType::MAGIC),
                 WeaponFactory::createHitEffectStrategy(WeaponType::MAGIC));

    CombatModifiers modifiers{1.0f, 1.0f};
    MagicHealEffect effect;
    CombatResult res = effect.apply(attacker, victim, modifiers, flute, combatSystem);

    EXPECT_TRUE(res.attackHappened);
    EXPECT_EQ(attacker.getMana(), initialMana);
    EXPECT_EQ(victim.getHp(), victim.getMaxHp());
}

// =========================================================================
// TEST 8: MagicHealEffect Objetivo No es Jugador (Monster)
// =========================================================================
TEST(CombatSystemTest, MagicHealEffectTargetNotPlayer) {
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, TestUtils::getTestServerConfig());

    Player attacker = TestUtils::makeTestPlayer(1);
    Monster monster = TestUtils::makeTestMonster(2);  // 🚀 Usamos el builder del helper unificado

    monster.setHealth(10);
    attacker.restoreMana();
    uint16_t initialMana = attacker.getMana();

    Weapon flute(4004, "Flauta elfa", 120, WeaponType::MAGIC, 0, 0, 3, 5,
                 WeaponFactory::createDeliveryStrategy(WeaponType::MAGIC),
                 WeaponFactory::createHitEffectStrategy(WeaponType::MAGIC));

    CombatModifiers modifiers{1.0f, 1.0f};
    MagicHealEffect effect;
    CombatResult res = effect.apply(attacker, monster, modifiers, flute, combatSystem);

    EXPECT_FALSE(res.attackHappened);
    EXPECT_EQ(attacker.getMana(), initialMana);
    EXPECT_EQ(monster.getHp(), 10);
}

// =========================================================================
// TEST 9: MagicHealEffect::isHeal() retorna true
// =========================================================================
TEST(CombatSystemTest, MagicHealEffect_isHeal_ReturnsTrue) {
    MagicHealEffect effect;
    EXPECT_TRUE(effect.isHeal());
}

// =========================================================================
// TEST 10: MeleeDamageEffect::isHeal() retorna false
// =========================================================================
TEST(CombatSystemTest, MeleeDamageEffect_isHeal_ReturnsFalse) {
    MeleeDamageEffect effect;
    EXPECT_FALSE(effect.isHeal());
}

// =========================================================================
// TEST 11: applyHealEffect propaga el flag isHeal en CombatResult
// =========================================================================
TEST(CombatSystemTest, ApplyHealEffect_SetsIsHealFlag) {
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, TestUtils::getTestServerConfig());

    Player victim = TestUtils::makeTestPlayer(1);
    victim.receiveDamage(10);
    ASSERT_LT(victim.getHp(), victim.getMaxHp());

    CombatResult res = combatSystem.applyHealEffect(victim);

    EXPECT_TRUE(res.attackHappened);
    EXPECT_TRUE(res.isHeal);
    EXPECT_EQ(victim.getHp(), victim.getMaxHp());
}
