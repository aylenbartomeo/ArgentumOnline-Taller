#include "Weapon.h"

Weapon::Weapon(int id, const std::string& name, int minDamage, int maxDamage, WeaponType type, int attackRange,
               int manaCost)
    : id(id),
      minDamage(minDamage),
      maxDamage(maxDamage),
      name(name),
      type(type),
      attackRange(attackRange),
      manaCost(manaCost) {}

int Weapon::getId() const { return id; }
int Weapon::getMinDamage() const { return minDamage; }
int Weapon::getMaxDamage() const { return maxDamage; }
const std::string& Weapon::getName() const { return name; }
WeaponType Weapon::getType() const { return type; }
int Weapon::getAttackRange() const { return attackRange; }
int Weapon::getManaCost() const { return manaCost; }
