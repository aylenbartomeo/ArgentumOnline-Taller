#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "Item.h"

class EquipmentComponent;
class FormulaEngine;
class IAttackDelivery;
class IHitEffect;
class Player;
class Attackable;
class CombatSystem; 
struct CombatResult; 
struct CombatModifiers;

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
           int attackRange, int manaCost, std::unique_ptr<IAttackDelivery> delivery,
           std::unique_ptr<IHitEffect> hitEffect);

    int getMinDamage() const;
    int getMaxDamage() const;
    WeaponType getType() const;
    int getAttackRange() const;
    int getManaCost() const;
    bool is_wearable() const override { return true; }
    void equip_on(EquipmentComponent& equipment, uint8_t slotIndex) const override;
    bool isMagic() const override;

    CombatResult deliver(Attackable& attacker, Attackable& target, const CombatModifiers& modifiers,
                         CombatSystem& combatSystem) const;

    CombatResult applyEffect(Player& attacker, Attackable& target, const CombatModifiers& modifiers,
                             CombatSystem& combatSystem) const;

    IHitEffect* getHitEffect() const { return hitEffectStrategy.get(); }
};
