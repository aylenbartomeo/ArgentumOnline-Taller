#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

struct BankSlot {
    uint32_t item_id{0};
    uint16_t amount{0};

    bool is_empty() const { return item_id == 0 || amount == 0; }
    void clear() {
        item_id = 0;
        amount = 0;
    }
};

class GlobalBank {
public:
    static constexpr size_t BANK_SIZE = 40;  // Capacidad fija de la bóveda bancaria

    struct BankAccount {
        uint32_t gold = 0;
        std::vector<BankSlot> slots;

        BankAccount(): gold(0), slots(BANK_SIZE) {}  // Inicializa los 40 slots vacíos
    };

private:
    std::unordered_map<uint32_t, BankAccount> accounts;

public:
    GlobalBank() = default;

    // --- GESTIÓN DE ORO ---
    void depositGold(uint32_t playerId, uint32_t amount) { accounts[playerId].gold += amount; }

    bool withdrawGold(uint32_t playerId, uint32_t amount) {
        auto it = accounts.find(playerId);
        if (it == accounts.end() || it->second.gold < amount)
            return false;
        it->second.gold -= amount;
        return true;
    }

    uint32_t getBankGold(uint32_t playerId) { return accounts[playerId].gold; }

    // --- GESTIÓN DE ÍTEMS CON APILAMIENTO ---
    // Retorna true si pudo depositar, false si el banco está lleno
    bool depositItem(uint32_t playerId, uint32_t itemId, uint16_t amount) {
        auto& account = accounts[playerId];

        // 1. Intentar encontrar un slot existente del mismo ítem para apilar
        auto itStack = std::find_if(
                account.slots.begin(), account.slots.end(),
                [itemId](const auto& slot) { return !slot.is_empty() && slot.item_id == itemId; });

        if (itStack != account.slots.end()) {
            itStack->amount += amount;  // Modificamos directo sobre el iterador
            return true;
        }

        // 2. Si no existe, intentar encontrar el primer slot libre
        auto itEmpty = std::find_if(account.slots.begin(), account.slots.end(),
                                    [](const auto& slot) { return slot.is_empty(); });

        if (itEmpty != account.slots.end()) {
            itEmpty->item_id = itemId;
            itEmpty->amount = amount;
            return true;
        }

        return false;  // Bóveda totalmente llena
    }

    // Retorna la cantidad realmente retirada (o 0 si no había)
    uint16_t withdrawItemById(uint32_t playerId, uint32_t itemId, uint16_t amountToWithdraw) {
        auto it = accounts.find(playerId);
        if (it == accounts.end())
            return 0;

        auto& account = it->second;

        // Buscamos el slot que contiene ese ID
        for (auto& slot: account.slots) {
            if (!slot.is_empty() && slot.item_id == itemId) {
                uint16_t actualWithdrawn = std::min(slot.amount, amountToWithdraw);

                slot.amount -= actualWithdrawn;
                if (slot.amount == 0) {
                    slot.clear();  // Si se quedó sin nada, liberamos el slot del banco
                }
                return actualWithdrawn;
            }
        }
        return 0;
    }

    // --- PERSISTENCIA ---
    const std::unordered_map<uint32_t, BankAccount>& getAllAccounts() const { return accounts; }

    void restoreAccount(uint32_t playerId, uint32_t gold,
                        const std::vector<BankSlot>& slotsToRestore) {
        auto& account = accounts[playerId];
        account.gold = gold;
        for (size_t i = 0; i < std::min(slotsToRestore.size(), BANK_SIZE); ++i) {
            account.slots[i] = slotsToRestore[i];
        }
    }
};
