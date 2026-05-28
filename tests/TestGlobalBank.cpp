#include <gtest/gtest.h>
#include "GlobalBank.h"

// =========================================================================
// TEST 1: DEPÓSITO Y EXTRACCIÓN DE ORO EXITOSA
// =========================================================================
TEST(GlobalBankTest, Bank_HandlesGoldLifecycleCorrectly) {
    GlobalBank banco;
    uint32_t playerId = 42;

    // 1. Depositamos en una cuenta inexistente (se debe crear automáticamente con 0)
    banco.depositGold(playerId, 1000u);
    banco.depositGold(playerId, 500u); // Sumamos más oro a la misma cuenta

    // Verificamos el saldo total depositado
    EXPECT_EQ(banco.getBankGold(playerId), 1500u);

    // 2. Intentamos retirar una cantidad válida
    EXPECT_TRUE(banco.withdrawGold(playerId, 1200u));
    EXPECT_EQ(banco.getBankGold(playerId), 300u);

    // 3. Si intentamos retirar más de lo que nos queda (300 restantes), debe fallar
    EXPECT_FALSE(banco.withdrawGold(playerId, 400u));

    // 4. Retiramos el remanente exacto
    EXPECT_TRUE(banco.withdrawGold(playerId, 300u));
    EXPECT_EQ(banco.getBankGold(playerId), 0u);
}

// =========================================================================
// TEST 2: RETIRO DE ORO EN CUENTA INEXISTENTE DEVUELVE FALSE
// =========================================================================
TEST(GlobalBankTest, Bank_WithdrawGoldFromNonExistentAccountReturnsFalse) {
    GlobalBank banco;
    uint32_t playerInexistente = 999;

    // Intentar sacar oro de alguien que nunca operó no debe romper el mapa
    EXPECT_FALSE(banco.withdrawGold(playerInexistente, 50u));
}

// =========================================================================
// TEST 3: APILAMIENTO DE ÍTEMS EN EL MISMO SLOT
// =========================================================================
TEST(GlobalBankTest, Bank_HandlesItemApilationCorrectly) {
    GlobalBank banco;
    uint32_t playerId = 55;

    // 1. Depositamos 5 pociones de vida (ID: 7004)
    EXPECT_TRUE(banco.depositItem(playerId, 7004u, 5));

    // 2. Depositamos 10 pociones más del mismo ID (deben apilarse juntas)
    EXPECT_TRUE(banco.depositItem(playerId, 7004u, 10));

    // 3. Retiramos parcialmente 3 pociones de ese slot
    uint16_t ret1 = banco.withdrawItemById(playerId, 7004u, 3);
    EXPECT_EQ(ret1, 3);

    // 4. Retiramos en exceso (quedan 15 - 3 = 12 pociones)
    uint16_t ret2 = banco.withdrawItemById(playerId, 7004u, 50);
    EXPECT_EQ(ret2, 12); // Solo nos debe entregar las 12 restantes

    // 5. Una nueva extracción debe dar 0 porque el slot se limpió/vació
    EXPECT_EQ(banco.withdrawItemById(playerId, 7004u, 1), 0);
}

// =========================================================================
// TEST 4: MANEJO DE MÚLTIPLES ÍTEMS DISTINTOS EN SLOTS SEPARADOS
// =========================================================================
TEST(GlobalBankTest, Bank_HandlesMultipleDifferentItems) {
    GlobalBank banco;
    uint32_t playerId = 100;

    // Depositamos dos ítems con IDs diferentes (ocupan slots separados en la grilla)
    EXPECT_TRUE(banco.depositItem(playerId, 4001u, 1));  // Espada
    EXPECT_TRUE(banco.depositItem(playerId, 1001u, 1));  // Armadura

    // Retiramos la armadura primero
    uint16_t retArmadura = banco.withdrawItemById(playerId, 1001u, 1);
    EXPECT_EQ(retArmadura, 1);

    // Intentamos retirar otra armadura (ya no hay)
    EXPECT_EQ(banco.withdrawItemById(playerId, 1001u, 1), 0);

    // La espada debe seguir intacta en su respectivo slot
    uint16_t retEspada = banco.withdrawItemById(playerId, 4001u, 1);
    EXPECT_EQ(retEspada, 1);
}

// =========================================================================
// TEST 5: EXTRAER ÍTEM INEXISTENTE DEVUELVE CERO
// =========================================================================
TEST(GlobalBankTest, Bank_WithdrawNonExistentItemReturnsZero) {
    GlobalBank banco;
    uint32_t playerId = 77;

    // Depositamos algo para que la cuenta exista en el mapa
    banco.depositItem(playerId, 1001u, 1);

    // Intentamos sacar un ID que jamás se guardó
    uint16_t cantidad = banco.withdrawItemById(playerId, 9999u, 1);
    EXPECT_EQ(cantidad, 0);
}