#include "server/src/model/items/Shield.h"

#include <utility>

#include "../components/EquipmentComponent.h"

Shield::Shield(const int id, std::string name, const int price, const int minDefense,
               const int maxDefense):
        Armor(id, std::move(name), price, minDefense, maxDefense) {}

int Shield::getDefense() const { return rollDefense(); }

void Shield::equip_on(EquipmentComponent& equipment, uint8_t slotIndex) const {
    equipment.equipShield(this, slotIndex);
}
