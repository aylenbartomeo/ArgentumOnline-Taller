#include <gtest/gtest.h>
#include "../model/entities/components/BankComponent.h"
#include "../model/entities/components/InventoryComponent.h"

// ========================================================================
// 1. TEST DE TRANSACCIONES DE ORO
// ========================================================================
TEST(PlayerBankIntegrationTest, DepositAndWithdrawGoldSuccessfully) {
    // Inicializamos inventario con 20 slots, 5000 de límite seguro, 100k máx.
    InventoryComponent inventory(20, 5000, 100000);
    // Banco con 50 slots y 1 millón de oro máximo.
    BankComponent bank(50, 1000000);

    inventory.addGold(10000);
    ASSERT_EQ(inventory.getGold(), 10000); // (Ajustá el getter si se llama diferente)
    ASSERT_EQ(bank.getVaultedGold(), 0);

    // Depositamos 4000 monedas
    bool dep_ok = bank.depositGold(4000, inventory);
    EXPECT_TRUE(dep_ok);
    EXPECT_EQ(inventory.getGold(), 6000);
    EXPECT_EQ(bank.getVaultedGold(), 4000);

    // Retiramos 1500 monedas
    bool wit_ok = bank.withdrawGold(1500, inventory);
    EXPECT_TRUE(wit_ok);
    EXPECT_EQ(inventory.getGold(), 7500);
    EXPECT_EQ(bank.getVaultedGold(), 2500);
}

// ========================================================================
// 2. TEST DE DEPOSITAR ÍTEMS CON ÉXITO
// ========================================================================
TEST(PlayerBankIntegrationTest, DepositItemSuccessfullyAndCheckStacking) {
    InventoryComponent inventory(20, 5000, 100000);
    BankComponent bank(50, 1000000);

    // Metemos 100 Espadas de Plata (ID: 501) en el slot 0 del inventario
    inventory.addItem(501, 100);
    
    // Depositamos 40 del slot 0 al banco
    bool dep_ok = bank.depositItem(0, 40, inventory);
    EXPECT_TRUE(dep_ok);

    // En el inventario tienen que quedar 60
    auto inv_slot = inventory.inspectSlot(0);
    ASSERT_TRUE(inv_slot.has_value());
    EXPECT_EQ(inv_slot->amount, 60);

    // Si retiramos 20 de ese item_id, deberían volver al inventario
    bool wit_ok = bank.withdrawItem(501, 20, inventory);
    EXPECT_TRUE(wit_ok);

    // Volvemos a tener 80 en la mochila
    inv_slot = inventory.inspectSlot(0);
    EXPECT_EQ(inv_slot->amount, 80);
}

// ========================================================================
// 3. TEST ANTICORRUPCIÓN: RESPALDAR SI LA MOCHILA ESTÁ LLENA
// ========================================================================
TEST(PlayerBankIntegrationTest, WithdrawItemFailsIfInventoryIsFullAndDoesNotDupe) {
    // Le damos solo 1 slot al inventario para forzar el llenado rápido
    InventoryComponent inventory(1, 5000, 100000);
    BankComponent bank(50, 1000000);

    // 1. Simulamos que el banco ya tiene guardadas 50 Pociones Rojas (ID: 202)
    // Para no romper la encapsulación, hacemos el pasamanos legal:
    inventory.addItem(202, 50);
    bank.depositItem(0, 50, inventory); // El inventario quedó vacío (0 ítems)
    
    // 2. Ahora llenamos el único slot del inventario con OTRO ítem (ID: 999, Raíz de Elfo)
    inventory.addItem(999, 1); 

    // 3. INTENTO DE RETIRO DE POCIONES (Falla porque no hay slots libres para el ID 202)
    bool wit_ok = bank.withdrawItem(202, 10, inventory);
    
    EXPECT_FALSE(wit_ok); // Tiene que rebotar
    
    // VERIFICACIÓN CRÍTICA: El banco NO tuvo que haber perdido las pociones
    // (Podés verificarlo intentando retirarlo después de vaciar o sumando un getter en tu banco si querés)
    // Para comprobarlo sin getters, vaciamos el slot 0 del inventario:
    inventory.removeItem(0, 1); // Sacamos la raíz
    
    // Ahora que hay espacio, el retiro de las pociones congeladas en el banco debería funcionar
    EXPECT_TRUE(bank.withdrawItem(202, 10, inventory));
}