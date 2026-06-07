#include "server/src/model/items/BodyArmor.h"

#include <utility>

#include "../components/EquipmentComponent.h"

BodyArmor::BodyArmor(const int id, std::string name, const int minDefense, const int maxDefense,
                     const int price):
        Armor(id, std::move(name), price, minDefense, maxDefense) {}

int BodyArmor::getDefense() const { return rollDefense(); }

void BodyArmor::equip_on(EquipmentComponent& equipment, uint8_t slotIndex) const {
    equipment.equipBodyArmor(this, slotIndex);
}
