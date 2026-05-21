#include <gtest/gtest.h>
#include "../server/src/model/entities/PlayerMock.h"
#include "../server/src/model/items/Weapon.h"

// =======================================================================
// TESTS DE STATS
// =======================================================================
TEST(PlayerStatsAttributesTest, InitialAttributesAreCorrect) {
    PlayerMock player(1, "Elegido");
    
    EXPECT_EQ(player.getStats().getStrength(), 15);
    EXPECT_EQ(player.getStats().getConstitution(), 16);
    EXPECT_EQ(player.getStats().getLevel(), 1);
}

// =======================================================================
// TESTS DE INVENTORY
// =======================================================================

TEST(PlayerInventoryTest, AddAndInspectItemWorks) {
    PlayerMock player(1, "Clérigo");
    
    // Agregamos 50 Pociones Rojas (ID: 10)
    EXPECT_TRUE(player.getInventory().addItem(10, 50));
    
    auto slot = player.getInventory().inspectSlot(0);
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot->item_id, 10);
    EXPECT_EQ(slot->amount, 50);
}

TEST(PlayerInventoryTest, RemoveItemClearsSlotWhenEmpty) {
    PlayerMock player(1, "Guerrero");
    player.getInventory().addItem(5, 10); // 10 Flechas en slot 0
    
    uint16_t removed = player.getInventory().removeItem(0, 10);
    EXPECT_EQ(removed, 10);
    
    auto slot = player.getInventory().inspectSlot(0);
    EXPECT_FALSE(slot.has_value()); // El slot debe haber quedado vacío (std::nullopt)
}

TEST(PlayerInventoryTest, DropExcessGoldOnDeathMechanic) {
    PlayerMock player(1, "Banquero");
    // Límite seguro inicial: 5000
    
    player.getInventory().addGold(8000); // Tiene 8000 monedas en total
    
    // El jugador muere, debe tirar el excedente al piso
    uint32_t floor_gold = player.getInventory().dropExcessGold();
    
    EXPECT_EQ(floor_gold, 3000); // 8000 - 5000 = 3000 al suelo
    EXPECT_EQ(player.getInventory().getGold(), 5000); // Conserva su límite seguro
}

// ========================================================================
// TESTS DE EQUIPAMIENTO 
// ========================================================================

TEST(PlayerEquipmentIntegrationTest, PlayerStartsWithNoWeaponAndZeroDefense) {
    PlayerMock player(1, "Novato");

    // Verificamos que los punteros del componente arranquen en nullptr
    EXPECT_EQ(player.getEquipment().getWeapon(), nullptr);
    EXPECT_EQ(player.getEquipment().getDefense(), 0);
}

TEST(PlayerEquipmentIntegrationTest, EquippingRealWeaponSuccessfully) {
    PlayerMock player(1, "Guerrero");

    // Instanciamos una Espada de Plata real (ID: 501, Daño: 15-25, Rango: 1, Mana: 0)
    Weapon espada(501, "Espada de Plata", 15, 25, WeaponType::MELEE, 1, 0);

    // Equipamos a través de la interfaz genérica de Item (Polimorfismo + Visitor)
    uint32_t replaced_id = player.getEquipment().equipItem(&espada);

    // Al estar la mano vacía, no debería haber reemplazado ningún ítem (retorna 0)
    EXPECT_EQ(replaced_id, 0);

    // El componente de equipamiento ahora debe apuntar a nuestra espada real
    ASSERT_NE(player.getEquipment().getWeapon(), nullptr);
    EXPECT_EQ(player.getEquipment().getWeapon()->getId(), 501);
    EXPECT_EQ(player.getEquipment().getWeapon()->getName(), "Espada de Plata");
}

TEST(PlayerEquipmentIntegrationTest, EquippingNewWeaponReturnsOldWeaponId) {
    PlayerMock player(1, "Cazador");

    // Creamos dos armas reales
    Weapon arcoViejo(10, "Arco de Madera", 5, 10, WeaponType::RANGED, 4, 0);
    Weapon arcoNuevo(11, "Arco Compuesto", 12, 22, WeaponType::RANGED, 5, 0);

    // Equipamos el primer arco
    player.getEquipment().equipItem(&arcoViejo);

    // Equipamos el segundo arco sobre el mismo slot ocupado
    uint32_t replaced_id = player.getEquipment().equipItem(&arcoNuevo);

    // ¡Debe retornar el ID del arco viejo (10) para que el juego sepa qué devolver al inventario!
    EXPECT_EQ(replaced_id, 10);

    // El slot actual debe haber quedado actualizado con el arco nuevo
    EXPECT_EQ(player.getEquipment().getWeapon()->getId(), 11);
}

TEST(PlayerEquipmentIntegrationTest, EquippingNullptrDoesNothingAndReturnsZero) {
    PlayerMock player(1, "Monje");

    uint32_t result = player.getEquipment().equipItem(nullptr);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(player.getEquipment().getWeapon(), nullptr);
}

// =======================================================================
// TESTS DE STATE COMPONENT
// =======================================================================

TEST(PlayerStateTest, NormalPlayerCanDoEverything) {
    PlayerMock player(1, "Aventurero");
    
    EXPECT_TRUE(player.getState().canMove());
    EXPECT_TRUE(player.getState().canAttack());
    EXPECT_TRUE(player.getState().canUseItems());
    EXPECT_FALSE(player.getState().isGhost());
}

TEST(PlayerStateTest, GhostPlayerCannotAttackOrUseItems) {
    PlayerMock player(1, "Difunto");
    
    player.getState().die(); // Transición a GhostState
    
    // Verificamos filtros del estado Ghost
    EXPECT_TRUE(player.getState().isGhost());
    EXPECT_TRUE(player.getState().canMove());
    EXPECT_FALSE(player.getState().canAttack());
    EXPECT_FALSE(player.getState().canUseItems());
}

TEST(PlayerStateTest, MeditatingPlayerCannotMoveOrAttack) {
    PlayerMock player(1, "Monje");
    
    player.getState().startMeditating(); // Transición a MeditatingState
    
    // Verificamos filtros del estado Meditar
    EXPECT_TRUE(player.getState().isMeditating());
    EXPECT_FALSE(player.getState().canMove());
    EXPECT_FALSE(player.getState().canAttack());
    EXPECT_FALSE(player.getState().canUseItems()); 
    
    // Si lo cortamos, vuelve a la normalidad
    player.getState().stopMeditating();
    EXPECT_TRUE(player.getState().canMove());
}