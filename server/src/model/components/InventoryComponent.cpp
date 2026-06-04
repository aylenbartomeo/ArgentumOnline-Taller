#include "InventoryComponent.h"

#include <algorithm>

InventoryComponent::InventoryComponent(const InventoryConfig& config):
        slots(config.maxSlots, Slot{0, 0}),
        gold(config.gold),
        safe_gold_limit(config.initialSafeGold),
        max_gold(config.maxGold) {}

uint16_t InventoryComponent::addItem(uint32_t item_id, uint16_t amount, bool stackable) {
    if (amount == 0)
        return 0;
    if (item_id == 0)
        return amount;

    uint16_t remaining = amount;
    std::vector<size_t> empty_slots;

    // 1. Primera pasada: Stacking (Solo si es stackable)
    if (stackable) {
        for (size_t i = 0; i < slots.size(); ++i) {
            if (slots[i].item_id == item_id && slots[i].amount < Slot::MAX_STACK_SIZE) {
                uint16_t space_available = Slot::MAX_STACK_SIZE - slots[i].amount;
                uint16_t to_add = std::min(remaining, space_available);
                slots[i].amount += to_add;
                remaining -= to_add;

                if (remaining == 0)
                    return 0;  // Se ubicó todo con éxito
            } else if (slots[i].is_empty()) {
                empty_slots.push_back(i);  // Guardamos los vacíos por si sobra
            }
        }
    } else {
        // Si no es stackable, igual recopilamos los vacíos
        for (size_t i = 0; i < slots.size(); ++i) {
            if (slots[i].is_empty()) {
                empty_slots.push_back(i);
            }
        }
    }

    // 2. Segunda pasada: Ocupar slots vacíos
    for (size_t empty_idx: empty_slots) {
        slots[empty_idx].item_id = item_id;

        if (stackable) {
            uint16_t to_add = std::min(remaining, Slot::MAX_STACK_SIZE);
            slots[empty_idx].amount = to_add;
            remaining -= to_add;
            if (remaining == 0) {
                return 0;
            }
        } else {
            // No stackable: ocupa 1 slot por unidad
            slots[empty_idx].amount = 1;
            remaining--;
            if (remaining == 0) {
                return 0;
            }
        }
    }

    // Retorna lo que sobró y no se pudo guardar
    return remaining;
}

std::vector<Slot> InventoryComponent::dropAllItems() {
    std::vector<Slot> dropped_items;
    for (auto& slot: slots) {
        if (!slot.is_empty()) {
            dropped_items.push_back(slot);
            slot.clear();
        }
    }
    return dropped_items;
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
    if (amount == 0 || gold >= max_gold)
        return false;

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
