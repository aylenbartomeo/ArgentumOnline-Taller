#include "BankComponent.h"

BankComponent::BankComponent(uint8_t total_slots, uint32_t max_g)
    : vaulted_slots(total_slots, Slot{0, 0}),
      vaulted_gold(0),
      max_bank_gold(max_g) {}

bool BankComponent::depositGold(uint32_t amount, InventoryComponent& inventory) { return false; }

bool BankComponent::withdrawGold(uint32_t amount, InventoryComponent& inventory) { return false; }

bool BankComponent::depositItem(uint8_t inv_slot, uint16_t amount, InventoryComponent& inventory) {
    return false;
}

bool BankComponent::withdrawItem(uint32_t item_id, uint16_t amount, InventoryComponent& inventory) {
    return false;
}
