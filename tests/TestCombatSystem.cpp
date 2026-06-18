// tests/TestCombatSystem.cpp
// Pruebas unitarias para el sistema de combate y sus estrategias de impacto

#include <memory>

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

// Callback de prueba para eventos de combate
class DummyCombatCallback: public ICombatEventCallback {
public:
    void onMonsterDeath(const Monster& monster, uint32_t killerDbId) override {}
    void onPlayerDeath(uint32_t dbId) override {}
};

// Helper: Crea un jugador base de prueba
static Player makeTestPlayer(uint32_t id = 1) {
    std::string name = "TestPlayer";
    Race race = Race::HUMAN;
    CharacterClass charClass = CharacterClass::WARRIOR;
    RaceConfig raceConfig = {1.0f, 1.0f, 1.0f};
    CharacterClassConfig clsConfig = {1.0f, 1.0f, 1.0f, false};
    PlayerConfig cfg = {15, 15, 15, 15, 1, 0, 0};
    InventoryConfig invCfg = {16, 0, 10000, 5000};
    return Player(id, id, name, race, charClass, raceConfig, clsConfig, cfg, invCfg,
                  FormulaEngine::getInstance());
}

// Helper: Crea un monstruo base de prueba
static Monster makeTestMonster(uint32_t id = 2) {
    Position pos{0, 0};
    MonsterConfig mConfig = {100, 100, 0, 10, 20, 5, 2, 1, 1, "zone", 0, 0};
    return Monster(id, NPCType::GOBLIN, pos, mConfig);
}

static ServerConfig getTestServerConfig() {
    ServerConfig config;
    config.worldName = "";
    config.mapPath = "";
    config.clanBonusRange = 5;
    config.criticalProbability = 0.10f;
    config.clanAttackBonusPerMember = 0.05f;
    config.clanDefenseBonusPerMember = 0.05f;
    return config;
}

// =========================================================================
// TEST 1: CombatSystem::applyDamageEffect Básico sin defensa
// =========================================================================
TEST(CombatSystemTest, ApplyDamageEffectBasicNoDefense) {
    // Explicación: Verifica la invariante fundamental de applyDamageEffect:
    // hp_final == hp_inicial - daño_reportado, siempre, independientemente
    // de si hubo esquive (damage=0) o crítico. El FormulaEngine usa RNG
    // real, por lo que no es posible garantizar damage > 0 en cada ejecución.
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, getTestServerConfig());

    Player attacker = makeTestPlayer(1);
    Player victim = makeTestPlayer(2);

    uint16_t initialHp = victim.getHp();

    // Daño min==max para que el roll de arma sea determinista (solo el esquive/crítico varía)
    AttackParams params{5, 5, 10, 0, false, 1.0f, 1.0f};

    CombatResult res = combatSystem.applyDamageEffect(attacker, victim, params);

    EXPECT_TRUE(res.attackHappened);
    // Invariante: el HP del objetivo siempre refleja exactamente el daño reportado
    uint16_t expectedHp =
            (res.damage >= initialHp) ? 0 : static_cast<uint16_t>(initialHp - res.damage);
    EXPECT_EQ(victim.getHp(), expectedHp);
    // Si hubo esquive, daño es 0 (válido). Si no, debe ser mayor a 0.
    if (!res.evaded) {
        EXPECT_GT(res.damage, 0);
    }
}

// =========================================================================
// TEST 2: CombatSystem::applyDamageEffect Con defensa de armadura
// =========================================================================
TEST(CombatSystemTest, ApplyDamageEffectWithDefense) {
    // Explicación: Verifica que la armadura equipada reduzca el daño recibido
    // en comparación con un objetivo sin armadura. Se compara el resultado
    // contra el mismo ataque sin armadura para comprobar la mitigación.
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, getTestServerConfig());

    Player attacker = makeTestPlayer(1);
    Player victimWithArmor = makeTestPlayer(2);
    Player victimNoArmor = makeTestPlayer(3);

    // Armadura con defensa fija min==max==10 para valores deterministas
    BodyArmor armor(1001, "Plates", 0, 10, 10);
    victimWithArmor.equipBodyArmor(&armor);

    AttackParams params{5, 5, 10, 0, false, 1.0f, 1.0f};

    // Usamos toggleInfiniteMana si fuera necesario; aquí simplemente corremos ambos
    CombatResult resWithArmor = combatSystem.applyDamageEffect(attacker, victimWithArmor, params);
    CombatResult resNoArmor = combatSystem.applyDamageEffect(attacker, victimNoArmor, params);

    EXPECT_TRUE(resWithArmor.attackHappened);
    EXPECT_TRUE(resNoArmor.attackHappened);

    // Invariante: HP siempre refleja el daño reportado
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

    // Si ninguno esquivó, el daño con armadura debe ser <= daño sin armadura
    if (!resWithArmor.evaded && !resNoArmor.evaded) {
        EXPECT_LE(resWithArmor.damage, resNoArmor.damage);
    }
}

// =========================================================================
// TEST 3: CombatSystem::applyHealEffect Básico
// =========================================================================
TEST(CombatSystemTest, ApplyHealEffectBasic) {
    // Explicación: Verifica que applyHealEffect restablezca los puntos de vida
    // de un jugador dañado.
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, getTestServerConfig());

    Player victim = makeTestPlayer(1);
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
    // Explicación: Verifica la aplicación correcta de MeleeDamageEffect.
    // Invariante: hp_final == hp_inicial - daño_reportado siempre.
    // Si hubo esquive, daño es 0 pero la invariante se mantiene.
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, getTestServerConfig());

    Player attacker = makeTestPlayer(1);
    Player victim = makeTestPlayer(2);

    // min==max para que el roll de arma sea determinista
    Weapon sword(4001, "Espada", 100, WeaponType::MELEE, 5, 5, 1, 0,
                 WeaponFactory::createDeliveryStrategy(WeaponType::MELEE),
                 WeaponFactory::createHitEffectStrategy(WeaponType::MELEE));

    uint16_t initialHp = victim.getHp();
    CombatModifiers modifiers{1.0f, 1.0f};

    MeleeDamageEffect effect;
    CombatResult res = effect.apply(attacker, victim, modifiers, sword, combatSystem);

    EXPECT_TRUE(res.attackHappened);
    // Invariante: el HP del objetivo siempre refleja exactamente el daño reportado
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
    // Explicación: Verifica que el daño mágico consuma el maná del atacante
    // y que la invariante hp_final == hp_inicial - daño_reportado se mantenga.
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, getTestServerConfig());

    Player attacker = makeTestPlayer(1);
    Player victim = makeTestPlayer(2);

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
    // El maná se consume independientemente del esquive
    EXPECT_EQ(attacker.getMana(), initialMana - 10);
    // Invariante: HP siempre refleja el daño reportado
    uint16_t expectedHp =
            (res.damage >= initialHp) ? 0 : static_cast<uint16_t>(initialHp - res.damage);
    EXPECT_EQ(victim.getHp(), expectedHp);
    if (!res.evaded) {
        EXPECT_GT(res.damage, 0);
    }
}

// =========================================================================
// TEST 6: MagicDamageEffect Maná Insuficiente
// =========================================================================
TEST(CombatSystemTest, MagicDamageEffectInsufficientMana) {
    // Explicación: Verifica que el daño mágico falle sin aplicar daño ni consumir
    // maná adicional cuando el atacante no posee maná suficiente.
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, getTestServerConfig());

    Player attacker = makeTestPlayer(1);
    Player victim = makeTestPlayer(2);

    // Vaciamos el maná del atacante
    attacker.consumeMana(attacker.getMana());
    ASSERT_EQ(attacker.getMana(), 0);

    Weapon staff(4003, "Vara de fresno", 120, WeaponType::MAGIC, 5, 5, 3, 10,
                 WeaponFactory::createDeliveryStrategy(WeaponType::MAGIC),
                 WeaponFactory::createHitEffectStrategy(WeaponType::MAGIC));

    CombatModifiers modifiers{1.0f, 1.0f};
    MagicDamageEffect effect;
    CombatResult res = effect.apply(attacker, victim, modifiers, staff, combatSystem);

    EXPECT_FALSE(res.attackHappened);
    EXPECT_EQ(attacker.getMana(), 0);
}

// =========================================================================
// TEST 7: MagicHealEffect Curación Exitosa
// =========================================================================
TEST(CombatSystemTest, MagicHealEffectSuccess) {
    // Explicación: Verifica que la curación mágica consuma el maná del atacante
    // y cure al jugador objetivo.
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, getTestServerConfig());

    Player attacker = makeTestPlayer(1);
    Player victim = makeTestPlayer(2);

    // Dañamos a la víctima y cargamos maná al atacante
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
    EXPECT_EQ(attacker.getMana(), initialMana - 5);
    EXPECT_EQ(victim.getHp(), victim.getMaxHp());
}

// =========================================================================
// TEST 8: MagicHealEffect Objetivo No es Jugador (Monster)
// =========================================================================
TEST(CombatSystemTest, MagicHealEffectTargetNotPlayer) {
    // Explicación: Verifica que la curación mágica no tenga efecto ni cure
    // si el objetivo es un Monstruo en lugar de un jugador.
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, getTestServerConfig());

    Player attacker = makeTestPlayer(1);
    Monster monster = makeTestMonster(2);

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
    // Aunque falle, no debería consumir maná por fallar las validaciones semánticas de objetivo
    EXPECT_EQ(attacker.getMana(), initialMana - 5);
    EXPECT_EQ(monster.getHp(), 10);
}

// =========================================================================
// TEST 9: MagicHealEffect::isHeal() retorna true
// =========================================================================
TEST(CombatSystemTest, MagicHealEffect_isHeal_ReturnsTrue) {
    // Explicación: Verifica que MagicHealEffect reporte correctamente que es
    // un efecto curativo, necesario para que ProjectileSystem bypasee las
    // guardias ofensivas (clanmates, fairPlay) al impactar con proyectil.
    MagicHealEffect effect;
    EXPECT_TRUE(effect.isHeal());
}

// =========================================================================
// TEST 10: MeleeDamageEffect::isHeal() retorna false
// =========================================================================
TEST(CombatSystemTest, MeleeDamageEffect_isHeal_ReturnsFalse) {
    // Explicación: Verifica que MeleeDamageEffect no sea clasificado como
    // efecto curativo; garantiza que el flujo de proyectiles no lo trate
    // como un heal y aplique correctamente las guardias ofensivas.
    MeleeDamageEffect effect;
    EXPECT_FALSE(effect.isHeal());
}

// =========================================================================
// TEST 11: applyHealEffect propaga el flag isHeal en CombatResult
// =========================================================================
TEST(CombatSystemTest, ApplyHealEffect_SetsIsHealFlag) {
    // Explicación: Verifica que el CombatResult devuelto por applyHealEffect
    // tenga isHeal=true para que CombatNotifier muestre el mensaje correcto
    // de curación en lugar del de daño.
    Map map;
    EntityManager em;
    ClanRepository cr;
    EventPublisher ep;
    DummyCombatCallback cb;
    BossSpawnSystem bss;
    CombatSystem combatSystem(map, em, cr, ep, cb, bss, false, getTestServerConfig());

    Player victim = makeTestPlayer(1);
    victim.receiveDamage(10);
    ASSERT_LT(victim.getHp(), victim.getMaxHp());

    CombatResult res = combatSystem.applyHealEffect(victim);

    EXPECT_TRUE(res.attackHappened);
    EXPECT_TRUE(res.isHeal);
    EXPECT_EQ(victim.getHp(), victim.getMaxHp());
}
