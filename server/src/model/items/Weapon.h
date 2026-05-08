#ifndef WEAPON_H
#define WEAPON_H

#include <string>
// #include "item.h"

enum class WeaponType {
    MELEE,
    RANGED,
    MAGIC
};

class Weapon{
private:

    int id;
    int minDamage;
    int maxDamage;
    std::string name;
    WeaponType type;
    int manaCost;

public:
    Weapon(int id, const std::string& name, int minDamage, int maxDamage, WeaponType type, int manaCost = 0);
    int getId() const;
    int getMinDamage() const;
    int getMaxDamage() const;
    const std::string& getName() const;
    WeaponType getType() const;
    int getManaCost() const;

};

#endif
