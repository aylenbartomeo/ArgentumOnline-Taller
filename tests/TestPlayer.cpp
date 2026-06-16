#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "model/components/StatsComponent.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"

// Helper: crea un Player base para tests
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

// =========================================================================
// TEST 1: INICIALIZACIÓN DE ATRIBUTOS BÁSICOS
// =========================================================================
TEST(PlayerTest, Player_Initialization) {
    // Explicación: Verifica que los atributos iniciales del Player se carguen correctamente
    Player player = makeTestPlayer(42);
    EXPECT_EQ(player.getId(), 42u);
    EXPECT_EQ(player.getDbId(), 42u);
    EXPECT_EQ(player.getName(), "TestPlayer");
    EXPECT_EQ(player.getGold(), 0u);
    EXPECT_FALSE(player.isDead());
    EXPECT_EQ(player.getHp(), player.getMaxHp());
}

// =========================================================================
// TEST 2: RECEPCIÓN DE DAÑO Y MUERTE
// =========================================================================
TEST(PlayerTest, Player_DamageAndDeath) {
    // Explicación: Verifica la recepcion de daño, el estado de muerte y los limites de vida
    Player player = makeTestPlayer();
    uint16_t initialHp = player.getHp();

    // Daño menor que la vida
    player.receiveDamage(initialHp / 2);
    EXPECT_EQ(player.getHp(), initialHp - (initialHp / 2));
    EXPECT_FALSE(player.isDead());

    // Daño que supera la vida restante (provoca muerte)
    player.receiveDamage(initialHp);
    EXPECT_TRUE(player.isDead());
    EXPECT_EQ(player.getHp(), 0u);
}

// =========================================================================
// TEST 3: RESURRECCIÓN
// =========================================================================
TEST(PlayerTest, Player_Resurrection) {
    // Explicación: Verifica que al resucitar se restablezca la vida y cambie el estado de muerte
    Player player = makeTestPlayer();
    player.receiveDamage(player.getMaxHp());
    ASSERT_TRUE(player.isDead());

    player.resurrect();
    EXPECT_FALSE(player.isDead());
    EXPECT_EQ(player.getHp(), player.getMaxHp());
}

// =========================================================================
// TEST 4: INVENTARIO Y ORO
// =========================================================================
TEST(PlayerTest, Player_InventoryAndGoldOperations) {
    // Explicación: Verifica la adicion y remocion de oro e items en el inventario del jugador
    Player player = makeTestPlayer();

    // Adicion de oro (mayor al límite seguro de 5000)
    EXPECT_TRUE(player.addGold(6000));
    EXPECT_EQ(player.getGold(), 6000u);

    // Remocion de oro
    EXPECT_TRUE(player.removeGold(400));
    EXPECT_EQ(player.getGold(), 5600u);

    // Límite seguro de oro inicial para el player es 5000.
    // El oro actual es 5600, por lo que el exceso a tirar es 5600 - 5000 = 600.
    uint32_t droppedGold = player.dropExcessGold();
    EXPECT_EQ(droppedGold, 600u);
    EXPECT_EQ(player.getGold(), 5000u);

    // Añadir items directamente al inventario (sin ItemRegistry)
    EXPECT_TRUE(player.addItem(100, 5));
    auto slotOpt = player.inspectSlot(0);
    ASSERT_TRUE(slotOpt.has_value());
    EXPECT_EQ(slotOpt->item_id, 100u);
    EXPECT_EQ(slotOpt->amount, 5);

    // Remover item
    uint16_t removed = player.removeItem(0, 2);
    EXPECT_EQ(removed, 2);
    slotOpt = player.inspectSlot(0);
    ASSERT_TRUE(slotOpt.has_value());
    EXPECT_EQ(slotOpt->amount, 3);
}

// =========================================================================
// TEST 5: SIMULACIÓN DE MOVIMIENTO
// =========================================================================
TEST(PlayerTest, Player_Movement) {
    // Explicación: Verifica que tryMove retorne la posicion tentativa sin alterar la actual,
    // y que setPosition actualice la posicion del jugador
    Player player = makeTestPlayer();
    Position startPos = player.getPosition();

    // Intentar moverse en una direccion
    Position nextPos = player.tryMove(Movement::RIGHT);
    EXPECT_EQ(nextPos.x, startPos.x + 1);
    EXPECT_EQ(nextPos.y, startPos.y);

    // Verificar que la posicion del jugador no ha cambiado
    EXPECT_EQ(player.getPosition().x, startPos.x);

    // Establecer la nueva posicion
    player.setPosition(nextPos);
    EXPECT_EQ(player.getPosition().x, startPos.x + 1);
}

// =========================================================================
// TEST 6: MANÁ
// =========================================================================
TEST(PlayerTest, Player_ManaUsage) {
    // Explicación: Verifica el consumo y la restauracion de mana
    Player player = makeTestPlayer();
    uint16_t maxMana = player.getMaxMana();

    // Consumir mana disponible
    EXPECT_TRUE(player.consumeMana(maxMana / 2));
    EXPECT_EQ(player.getMana(), maxMana - (maxMana / 2));

    // Intentar consumir mas mana del disponible
    EXPECT_FALSE(player.consumeMana(maxMana));
    EXPECT_EQ(player.getMana(), maxMana - (maxMana / 2));  // No debe alterarse

    // Restaurar mana
    player.restoreMana();
    EXPECT_EQ(player.getMana(), maxMana);
}

// =========================================================================
// TEST 7: REGLAS DE COMBATE (NIVEL / PVP / NEWBIE)
// =========================================================================
TEST(PlayerTest, Player_CombatRules) {
    // Explicación: Verifica las restricciones de PvP basadas en nivel de novato (newbie) y
    // diferencias de nivel
    Player newbie = makeTestPlayer(1);   // Nivel 1
    Player victim1 = makeTestPlayer(2);  // Nivel 1

    // Regla 1: Un novato (newbie, nivel <= 12) no puede entrar en combate con otro jugador
    EXPECT_FALSE(newbie.canEngageInCombatWith(victim1));
    EXPECT_FALSE(victim1.canEngageInCombatWith(newbie));

    // Elevamos el nivel de los jugadores para que dejen de ser newbies
    // Agregamos mucha experiencia para forzar la subida de nivel > 12
    newbie.addExperience(1000000);  // Sube mucho mas alla del nivel 12
    victim1.addExperience(1000000);

    ASSERT_GT(newbie.getLevel(), 12);
    ASSERT_GT(victim1.getLevel(), 12);

    // Ahora que no son newbies, deberian poder combatir si tienen diferencia de nivel <= 10
    EXPECT_TRUE(newbie.canEngageInCombatWith(victim1));

    // Creamos un tercer jugador de nivel alto para verificar la diferencia de nivel
    Player highLevel = makeTestPlayer(3);
    highLevel.addExperience(10000000);  // Nivel muy alto

    // Verificar que la diferencia de nivel es > 10 y por lo tanto no pueden combatir
    ASSERT_GT(highLevel.getLevel() - victim1.getLevel(), 10);
    EXPECT_FALSE(victim1.canEngageInCombatWith(highLevel));
    EXPECT_FALSE(highLevel.canEngageInCombatWith(victim1));
}

// =========================================================================
// TEST 8: PÉRDIDA DE EXPERIENCIA POR MUERTE Y PROTECCIÓN DE NIVEL
// =========================================================================
TEST(PlayerTest, Player_ExperienceLossOnDeathAndFloorProtection) {
    // Explicación: Verifica que al morir se reduzca la experiencia del jugador,
    // pero garantiza que muertes consecutivas no fuercen un descenso de nivel
    // ni perforen el piso de EXP requerido para su nivel actual.
    Player player = makeTestPlayer();

    player.addExperience(10000); 
    uint8_t levelBeforeDeath = player.getLevel();
    uint32_t expBeforeDeath = player.getExp();

    ASSERT_GT(levelBeforeDeath, 1u);

    player.receiveDamage(player.getHp());
    ASSERT_TRUE(player.isDead());

    EXPECT_LT(player.getExp(), expBeforeDeath);
    EXPECT_EQ(player.getLevel(), levelBeforeDeath);

    // TEST DE ESTRÉS / LÍMITE: Muertes consecutivas para buscar el "Downgrade" de nivel
    // Lo revivimos y lo matamos repetidamente para drenarle toda la experiencia posible
    for (int i = 0; i < 5; ++i) {
        player.resurrect();
        player.receiveDamage(player.getHp());
    }

    EXPECT_EQ(player.getLevel(), levelBeforeDeath);

    // La experiencia debió estancarse exactamente en el piso requerido para su nivel actual.
    // El piso de experiencia es el límite requerido para superar el nivel anterior (level - 1).
    uint32_t expectedFloor = FormulaEngine::getInstance().calculate_level_up_limit(levelBeforeDeath - 1);
    EXPECT_EQ(player.getExp(), expectedFloor);
}
