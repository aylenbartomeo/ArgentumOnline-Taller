#include "server/src/model/items/Helmet.h"

#include <utility>

#include "../components/EquipmentComponent.h"

Helmet::Helmet(const int id, std::string name, const int price, const int minDefense,
               const int maxDefense):
        Armor(id, std::move(name), price, minDefense, maxDefense) {}

int Helmet::getDefense() const { return rollDefense(); }

void Helmet::equip_on(EquipmentComponent& equipment, uint8_t slotIndex) const {
    equipment.equipHelmet(this, slotIndex);
}
