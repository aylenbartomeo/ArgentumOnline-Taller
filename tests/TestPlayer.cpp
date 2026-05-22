#include <gtest/gtest.h>
#include "../server/src/model/entities/PlayerFactory.h"
#include "../server/src/model/entities/PlayerMock.h"
#include "../server/src/model/items/Weapon.h"

// =======================================================================
// FIXTURE DE BASE: Configuración limpia compartida por todos los tests
// =======================================================================
class PlayerTestFixture : public ::testing::Test {
protected:
    PlayerConfig base;
    RaceConfig race;
    CharacterClassConfig cls;

    void SetUp() override {
        // Configuramos datos de prueba fijos y controlados respetando CharacterConfig.h
        base = {
            15, // baseStrength
            15, // baseIntelligence
            15, // baseAgility
            15, // baseConstitution
            1,  // startingLevel
            0,  // startingExperience
            0   // startingGold
        }; 

        race = {
            1.0f, // lifeFactor
            1.0f, // manaFactor
            1.0f  // recoveryFactor
        };          

        cls = {
            1.0f,  // lifeFactor
            1.0f,  // manaFactor
            1.0f,  // meditationFactor
            false  // canUseMagic
        };
    }
};

// =======================================================================
// TESTS DE STATS
// =======================================================================

TEST_F(PlayerTestFixture, StatsInitialValuesCorrectlyCalculated) {
    // Instanciamos un jugador nivel 1
    PlayerMock player(1, "Héroe", race, cls, base);

    // Verificaciones iniciales básicas
    EXPECT_EQ(player.getStats().getLevel(), 1);
    EXPECT_EQ(player.getStats().getExp(), 0);

    // FormulaEngine: constitution(15) * class.lifeFactor(1.0) * race.lifeFactor(1.0) * level(1) = 15
    EXPECT_EQ(player.getStats().getMaxHp(), 15);
    EXPECT_EQ(player.getStats().getHp(), 15); // Nace con la vida al máximo
}

// =======================================================================
// TESTS DE INVENTORY (Usando el Fixture)
// =======================================================================

TEST_F(PlayerTestFixture, AddAndInspectItemWorks) {
    // Instanciamos respetando el nuevo constructor de producción que nos pasaste
    PlayerMock player(1, "Clérigo", race, cls, base);
    
    // Agregamos 50 Pociones Rojas (ID: 10)
    EXPECT_TRUE(player.getInventory().addItem(10, 50));
    
    auto slot = player.getInventory().inspectSlot(0);
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot->item_id, 10);
    EXPECT_EQ(slot->amount, 50);
}

TEST_F(PlayerTestFixture, RemoveItemClearsSlotWhenEmpty) {
    PlayerMock player(1, "Clérigo", race, cls, base);
    player.getInventory().addItem(5, 10); // 10 Flechas en slot 0
    
    uint16_t removed = player.getInventory().removeItem(0, 10);
    EXPECT_EQ(removed, 10);
    
    auto slot = player.getInventory().inspectSlot(0);
    EXPECT_FALSE(slot.has_value()); // El slot debe haber quedado vacío (std::nullopt)
}

TEST_F(PlayerTestFixture, DropExcessGoldOnDeathMechanic) {
    PlayerMock player(1, "Banquero", race, cls, base);

    // Límite seguro inicial configurado internamente: 5000
    player.getInventory().addGold(8000); // Tiene 8000 monedas en total
    
    // El jugador muere, debe tirar el excedente al piso
    uint32_t floor_gold = player.getInventory().dropExcessGold();
    
    EXPECT_EQ(floor_gold, 3000); // 8000 - 5000 = 3000 al suelo
    EXPECT_EQ(player.getInventory().getGold(), 5000); // Conserva su límite seguro
}

// ========================================================================
// TESTS DE EQUIPAMIENTO 
// ========================================================================

TEST_F(PlayerTestFixture, PlayerStartsWithNoWeaponAndZeroDefense) {
    PlayerMock player(1, "Novato", race, cls, base);

    EXPECT_EQ(player.getEquipment().getWeapon(), nullptr);
    EXPECT_EQ(player.getEquipment().getDefense(), 0);
}

TEST_F(PlayerTestFixture, EquippingRealWeaponSuccessfully) {
    PlayerMock player(1, "Guerrero", race, cls, base);

    // Instanciamos una Espada de Plata real
    Weapon espada(501, "Espada de Plata", 15, 25, WeaponType::MELEE, 1, 0);

    // Equipamos a través de la interfaz genérica
    uint32_t replaced_id = player.getEquipment().equipItem(&espada);

    EXPECT_EQ(replaced_id, 0);
    ASSERT_NE(player.getEquipment().getWeapon(), nullptr);
    EXPECT_EQ(player.getEquipment().getWeapon()->getId(), 501);
    EXPECT_EQ(player.getEquipment().getWeapon()->getName(), "Espada de Plata");
}

TEST_F(PlayerTestFixture, EquippingNewWeaponReturnsOldWeaponId) {
    PlayerMock player(1, "Cazador", race, cls, base);

    Weapon arcoViejo(10, "Arco de Madera", 5, 10, WeaponType::RANGED, 4, 0);
    Weapon arcoNuevo(11, "Arco Compuesto", 12, 22, WeaponType::RANGED, 5, 0);

    player.getEquipment().equipItem(&arcoViejo);
    uint32_t replaced_id = player.getEquipment().equipItem(&arcoNuevo);

    EXPECT_EQ(replaced_id, 10); // Retorna el viejo para devolverlo al inventario
    EXPECT_EQ(player.getEquipment().getWeapon()->getId(), 11);
}

TEST_F(PlayerTestFixture, EquippingNullptrDoesNothingAndReturnsZero) {
    PlayerMock player(1, "Monje", race, cls, base);

    uint32_t result = player.getEquipment().equipItem(nullptr);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(player.getEquipment().getWeapon(), nullptr);
}

// =======================================================================
// TESTS DE STATE COMPONENT
// =======================================================================

TEST_F(PlayerTestFixture, NormalPlayerCanDoEverything) {
    PlayerMock player(1, "Carlos", race, cls, base);
    
    EXPECT_TRUE(player.getState().canMove());
    EXPECT_TRUE(player.getState().canAttack());
    EXPECT_TRUE(player.getState().canUseItems());
    EXPECT_FALSE(player.getState().isGhost());
}

TEST_F(PlayerTestFixture, GhostPlayerCannotAttackOrUseItems) {
    PlayerMock player(1, "Difunto", race, cls, base);
    
    player.getState().die(); // Transición a GhostState
    
    EXPECT_TRUE(player.getState().isGhost());
    EXPECT_TRUE(player.getState().canMove());
    EXPECT_FALSE(player.getState().canAttack());
    EXPECT_FALSE(player.getState().canUseItems());
}

TEST_F(PlayerTestFixture, MeditatingPlayerCannotMoveOrAttack) {
    PlayerMock player(1, "Monje", race, cls, base);
    
    player.getState().startMeditating(); // Transición a MeditatingState
    
    EXPECT_TRUE(player.getState().isMeditating());
    EXPECT_FALSE(player.getState().canMove());
    EXPECT_FALSE(player.getState().canAttack());
    EXPECT_FALSE(player.getState().canUseItems()); 
    
    player.getState().stopMeditating();
    EXPECT_TRUE(player.getState().canMove());
}