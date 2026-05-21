#include "server/src/model/items/Helmet.h"

#include <utility>

#include "../entities/components/EquipmentComponent.h"

Helmet::Helmet(const int id, std::string name, const int minDefense, const int maxDefense):
        Armor(id, std::move(name), minDefense, maxDefense) {}

int Helmet::getDefense() const { return rollDefense(); }

uint32_t Helmet::equip_on(EquipmentComponent& equipment) const { return equipment.equipHelmet(this); }
