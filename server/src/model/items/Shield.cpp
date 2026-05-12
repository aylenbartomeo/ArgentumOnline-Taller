#include "server/src/model/items/Shield.h"

#include <utility>

#include "server/src/model/items/Equipment.h"

Shield::Shield(const int id, std::string name, const int minDefense, const int maxDefense):
        Armor(id, std::move(name), minDefense, maxDefense) {}

int Shield::getDefense() const { return rollDefense(); }

uint32_t Shield::equip_on(Equipment& equipment) const { return equipment.equip_shield(this); }
