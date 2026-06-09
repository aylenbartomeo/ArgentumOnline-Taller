#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "Item.h"

class EquipmentComponent;
class FormulaEngine;
class IAttackDelivery;
class IHitEffect;

enum class WeaponType { MELEE, RANGED, MAGIC };

class Weapon: public Item {
private:
    int minDamage;
    int maxDamage;
    WeaponType type;
    int attackRange;
    int manaCost;

    std::unique_ptr<IAttackDelivery> deliveryStrategy;
    std::unique_ptr<IHitEffect> hitEffectStrategy;

public:
    ~Weapon() override;

    Weapon(int id, std::string name, int price, WeaponType type, int minDamage, int maxDamage,
           int attackRange = 1, int manaCost = 0);

    int getMinDamage() const;
    int getMaxDamage() const;
    WeaponType getType() const;
    int getAttackRange() const;
    int getManaCost() const;
    bool is_wearable() const override { return true; }
    void equip_on(EquipmentComponent& equipment, uint8_t slotIndex) const override;
    bool isMagic() const override;

    IAttackDelivery* getDelivery() const { return deliveryStrategy.get(); }
    IHitEffect* getHitEffect() const { return hitEffectStrategy.get(); }
};
