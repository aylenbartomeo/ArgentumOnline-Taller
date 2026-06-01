#include "Weapon.h"

#include <stdexcept>

#include "../components/EquipmentComponent.h"
#include "model/FormulaEngine.h"

Weapon::Weapon(int id, std::string name, int price, WeaponType type, int minDamage, int maxDamage,
               int attackRange, int manaCost):
        Item(id, std::move(name), price),
        minDamage(minDamage),
        maxDamage(maxDamage),
        type(type),
        attackRange(attackRange),
        manaCost(manaCost) {

    // Validate weapon-specific parameters (name is already validated by Item)
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

int Weapon::getMinDamage() const { return minDamage; }
int Weapon::getMaxDamage() const { return maxDamage; }
WeaponType Weapon::getType() const { return type; }
int Weapon::getAttackRange() const { return attackRange; }
int Weapon::getManaCost() const { return manaCost; }

uint16_t Weapon::calculateDamage(uint16_t attackPower) const {
    return FormulaEngine::getInstance().calculate_base_damage(
            attackPower, static_cast<uint16_t>(this->minDamage),
            static_cast<uint16_t>(this->maxDamage));
}

uint32_t Weapon::equip_on(EquipmentComponent& equipment) const {
    return equipment.equipWeapon(this);
}
