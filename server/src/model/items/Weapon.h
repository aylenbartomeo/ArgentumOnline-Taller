#ifndef WEAPON_H
#define WEAPON_H

#include <cstdint>
#include <string>

#include "Item.h"

class FormulaEngine;

enum class WeaponType { MELEE, RANGED, MAGIC };

class Weapon: public Item {
private:
    int id;
    int minDamage;
    int maxDamage;
    std::string name;
    WeaponType type;
    int attackRange;
    int manaCost;

public:
    ~Weapon() override = default;

    Weapon(int id, const std::string& name, int minDamage, int maxDamage, WeaponType type,
           int attackRange, int manaCost = 0);
    int getId() const override;
    int getMinDamage() const;
    int getMaxDamage() const;
    const std::string& getName() const override;
    WeaponType getType() const;
    int getAttackRange() const;
    int getManaCost() const;
    uint16_t calculateDamage(uint16_t attackPower, const FormulaEngine& formulas) const;
    bool is_wearable() const override { return true; }
    uint32_t equip_on(Equipment& equipment) const override;
};

#endif
