#include "server/src/model/items/BodyArmor.h"

#include <utility>

#include "../components/EquipmentComponent.h"

BodyArmor::BodyArmor(const int id, std::string name, const int minDefense, const int maxDefense, const int price):
    Armor(id, std::move(name), price, minDefense, maxDefense) {}

int BodyArmor::getDefense() const { return rollDefense(); }

uint32_t BodyArmor::equip_on(EquipmentComponent& equipment) const {
    return equipment.equipBodyArmor(this);
}
