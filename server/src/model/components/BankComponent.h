#ifndef BANK_COMPONENT_H
#define BANK_COMPONENT_H

#include <algorithm>
#include <cstdint>
#include <vector>

#include "InventoryComponent.h"

class BankComponent {
private:
    std::vector<Slot> vaulted_slots;  // Ítems guardados en la caja fuerte
    uint32_t vaulted_gold;            // Oro total guardado en el banco
    uint32_t
            max_bank_gold;  // Límite máximo de oro en el banco (p.ej. 2^32 - 1 o un tope del juego)

public:
    // Arranca en 0 oro y con un tamaño inicial de slots de banco (ej. 50 slots)
    explicit BankComponent(uint8_t total_slots = 50, uint32_t max_g = 100000000);
    ~BankComponent() = default;

    // Bloqueamos copia para seguridad transaccional
    BankComponent(const BankComponent&) = delete;
    BankComponent& operator=(const BankComponent&) = delete;
    BankComponent(BankComponent&&) = default;
    BankComponent& operator=(BankComponent&&) = default;

    // ========================================================================
    // TRANSACCIONES DE ORO
    // ========================================================================
    bool depositGold(uint32_t amount, InventoryComponent& inventory);
    bool withdrawGold(uint32_t amount, InventoryComponent& inventory);

    uint32_t getVaultedGold() const { return vaulted_gold; }

    // ========================================================================
    // GESTIÓN DE ÍTEMS (Estructura base)
    // ========================================================================
    bool depositItem(uint8_t inv_slot, uint16_t amount, InventoryComponent& inventory);
    bool withdrawItem(uint32_t item_id, uint16_t amount, InventoryComponent& inventory);
};

#endif  // BANK_COMPONENT_H
