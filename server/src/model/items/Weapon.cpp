#include "Weapon.h"

#include <stdexcept>

#include "model/FormulaEngine.h"
#include "server/src/model/items/Equipment.h"

Weapon::Weapon(int id, const std::string& name, int minDamage, int maxDamage, WeaponType type,
               int attackRange, int manaCost):
        id(id),
        minDamage(minDamage),
        maxDamage(maxDamage),
        name(name),
        type(type),
        attackRange(attackRange),
        manaCost(manaCost) {
    if (id < 0) {
        throw std::invalid_argument("Weapon id cannot be negative");
    }

    if (name.empty()) {
        throw std::invalid_argument("Weapon name cannot be empty");
    }

    if (minDamage < 0) {
        throw std::invalid_argument("Weapon minimum damage cannot be negative");
    }

    if (minDamage > maxDamage) {
        throw std::invalid_argument("Weapon minimum damage cannot exceed maximum damage");
    }

    if (attackRange < 1) {
        throw std::invalid_argument("Weapon attack range must be positive");
    }

    if (manaCost < 0) {
        throw std::invalid_argument("Weapon mana cost cannot be negative");
    }
}

int Weapon::getId() const { return id; }
int Weapon::getMinDamage() const { return minDamage; }
int Weapon::getMaxDamage() const { return maxDamage; }
const std::string& Weapon::getName() const { return name; }
WeaponType Weapon::getType() const { return type; }
int Weapon::getAttackRange() const { return attackRange; }
int Weapon::getManaCost() const { return manaCost; }

uint16_t Weapon::calculateDamage(uint16_t attackPower) const {
    return FormulaEngine::getInstance().calculate_base_damage(attackPower, static_cast<uint16_t>(this->minDamage),
                                                              static_cast<uint16_t>(this->maxDamage));
}

uint32_t Weapon::equip_on(Equipment& equipment) const { return equipment.equip_weapon(this); }
