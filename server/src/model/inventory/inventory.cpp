#include "server/src/model/inventory/inventory.h"

#include <algorithm>

Inventory::Inventory(const InventoryConfig& config, uint32_t initial_safe_gold):
        safe_gold_limit(initial_safe_gold) {
    slots.resize(config.maxSlots);
}

bool Inventory::add_item(uint32_t item_id, uint16_t amount) {
    if (amount == 0 || item_id == 0)
        return false;

    std::optional<size_t> first_empty_index = std::nullopt;

    for (size_t i = 0; i < slots.size(); ++i) {
        if (slots[i].item_id == item_id) {
            if (UINT16_MAX - slots[i].amount < amount) {
                return false;
            }
            slots[i].amount += amount;
            return true;
        }
        if (!first_empty_index.has_value() && slots[i].is_empty()) {
            first_empty_index = i;
        }
    }
    if (first_empty_index.has_value()) {
        slots[*first_empty_index].item_id = item_id;
        slots[*first_empty_index].amount = amount;
        return true;
    }

    return false;
}

uint16_t Inventory::remove_item(uint8_t slot_index, uint16_t amount) {
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

std::optional<Slot> Inventory::inspect_slot(uint8_t slot_index) const {
    if (slot_index >= slots.size() || slots[slot_index].is_empty()) {
        return std::nullopt;
    }
    return slots[slot_index];
}

void Inventory::add_gold(uint32_t amount) {
    if (UINT32_MAX - gold < amount) {
        gold = UINT32_MAX;
    } else {
        gold += amount;
    }
}

bool Inventory::remove_gold(uint32_t amount) {
    if (gold < amount)
        return false;
    gold -= amount;
    return true;
}

void Inventory::update_safe_limit(uint32_t new_limit) { safe_gold_limit = new_limit; }

uint32_t Inventory::drop_excess_gold() {
    if (gold <= safe_gold_limit) {
        return 0;
    }
    uint32_t dropped_amount = gold - safe_gold_limit;
    gold = safe_gold_limit;
    return dropped_amount;
}
