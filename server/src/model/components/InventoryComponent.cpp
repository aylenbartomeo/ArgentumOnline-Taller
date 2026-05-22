#include "InventoryComponent.h"

#include <algorithm>

InventoryComponent::InventoryComponent(const InventoryConfig& config, uint32_t initial_safe_gold)
    : slots(config.maxSlots, Slot{0, 0}),
      gold(0),
      safe_gold_limit(initial_safe_gold),
      max_gold(config.maxGold) {}

bool InventoryComponent::addItem(uint32_t item_id, uint16_t amount) {
    if (amount == 0 || item_id == 0)
        return false;

    uint16_t remaining = amount;
    std::vector<size_t> empty_slots;

    // 1. Primer pasada: Intentamos llenar slots del mismo ítem que tengan espacio
    for (size_t i = 0; i < slots.size(); ++i) {
        if (slots[i].item_id == item_id) {
            uint16_t space_available = UINT16_MAX - slots[i].amount;
            if (space_available > 0) {
                uint16_t to_add = std::min(remaining, space_available);
                slots[i].amount += to_add;
                remaining -= to_add;
                
                if (remaining == 0) return true; // Se ubicó todo con éxito
            }
        } else if (slots[i].is_empty()) {
            empty_slots.push_back(i); // Guardamos los vacíos por si sobra
        }
    }

    // 2. Segunda pasada: Si todavía queda remanente, usamos los slots vacíos
    for (size_t empty_idx : empty_slots) {
        slots[empty_idx].item_id = item_id;
        slots[empty_idx].amount = remaining; // En este punto 'remaining' entra seguro en un slot vacío
        return true; 
    }

    // Si llegamos acá y 'remaining < amount', significa que se guardó ALGO pero no todo.
    // Para simplificar tu contrato actual, si no entra TODO el fardo devolvemos false 
    // (o podés revertir el estado si preferís transacciones atómicas).
    return remaining == 0;
}

uint16_t InventoryComponent::removeItem(uint8_t slot_index, uint16_t amount) {
    if (slot_index >= slots.size())
        return 0;

    Slot& slot = slots[slot_index];
    if (slot.is_empty() || amount == 0)
        return 0;

    uint16_t removed = std::min(slot.amount, amount);
    slot.amount -= removed;

    if (slot.amount == 0) {
        slot.clear();
    }

    return removed;
}

std::optional<Slot> InventoryComponent::inspectSlot(uint8_t slot_index) const {
    if (slot_index >= slots.size() || slots[slot_index].is_empty()) {
        return std::nullopt;
    }
    return slots[slot_index];
}

bool InventoryComponent::addGold(uint32_t amount) {
    if (amount == 0 || gold >= max_gold) return false;

    // Protegemos contra desbordamiento usando el tope máximo del juego
    if (max_gold - gold < amount) {
        gold = max_gold;
    } else {
        gold += amount;
    }
    return true;
}

bool InventoryComponent::removeGold(uint32_t amount) {
    if (gold < amount)
        return false;
    gold -= amount;
    return true;
}

void InventoryComponent::updateSafeLimit(uint32_t new_limit) { safe_gold_limit = new_limit; }

uint32_t InventoryComponent::dropExcessGold() {
    if (gold <= safe_gold_limit) {
        return 0;
    }
    uint32_t dropped_amount = gold - safe_gold_limit;
    gold = safe_gold_limit;
    return dropped_amount;
}
