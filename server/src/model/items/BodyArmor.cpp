#include "server/src/model/items/BodyArmor.h"

#include <utility>

#include "server/src/model/items/Equipment.h"

BodyArmor::BodyArmor(const int id, std::string name, const int minDefense, const int maxDefense):
        Armor(id, std::move(name), minDefense, maxDefense) {}

int BodyArmor::getDefense() const { return rollDefense(); }

uint32_t BodyArmor::equip_on(Equipment& equipment) const {
    return equipment.equip_body_armor(this);
}
