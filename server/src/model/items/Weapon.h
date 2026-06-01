#pragma once

#include <cstdint>
#include <string>

#include "Item.h"

class EquipmentComponent;
class FormulaEngine;

enum class WeaponType { MELEE, RANGED, MAGIC };

class Weapon: public Item {
private:
    int minDamage;
    int maxDamage;
    WeaponType type;
    int attackRange;
    int manaCost;

public:
    ~Weapon() override = default;

    Weapon(int id, std::string name, int price, WeaponType type, int minDamage, int maxDamage,
           int attackRange = 1, int manaCost = 0);

    bool isMagic() const override;
    int getMinDamage() const;
    int getMaxDamage() const;
    WeaponType getType() const;
    int getAttackRange() const;
    int getManaCost() const;
    uint16_t calculateDamage(uint16_t attackPower) const;
    bool is_wearable() const override { return true; }
    uint32_t equip_on(EquipmentComponent& equipment) const override;
};
