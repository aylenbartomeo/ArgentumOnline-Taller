#include "Weapon.h"

#include <stdexcept>
#include <utility>

#include "../components/EquipmentComponent.h"
#include "../interfaces/CombatStrategies.h"

#include "FormulaEngine.h"

Weapon::Weapon(int id, std::string name, int price, WeaponType type, int minDamage, int maxDamage,
               int attackRange, int manaCost, std::unique_ptr<IAttackDelivery> delivery,
               std::unique_ptr<IHitEffect> hitEffect):
        Item(id, std::move(name), price),
        minDamage(minDamage),
        maxDamage(maxDamage),
        type(type),
        attackRange(attackRange),
        manaCost(manaCost),
        deliveryStrategy(std::move(delivery)),
        hitEffectStrategy(std::move(hitEffect)) {

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

    // Strategies are now injected by the factory.
}

int Weapon::getMinDamage() const { return minDamage; }
int Weapon::getMaxDamage() const { return maxDamage; }
WeaponType Weapon::getType() const { return type; }
int Weapon::getAttackRange() const { return attackRange; }
int Weapon::getManaCost() const { return manaCost; }

void Weapon::equip_on(EquipmentComponent& equipment, uint8_t slotIndex) const {
    equipment.equipWeapon(this, slotIndex);
}

bool Weapon::isMagic() const {
    if (type == WeaponType::MAGIC) {
        return true;
    }
    return false;
}

CombatResult Weapon::deliver(Attackable& attacker, Attackable& target,
                             const CombatModifiers& modifiers, CombatSystem& combatSystem) const {
    if (!deliveryStrategy) {
        return CombatResult();
    }
    return deliveryStrategy->deliver(attacker, target, modifiers, *this, combatSystem);
}

CombatResult Weapon::applyEffect(Player& attacker, Attackable& target,
                                 const CombatModifiers& modifiers,
                                 CombatSystem& combatSystem) const {
    if (!hitEffectStrategy) {
        CombatResult defaultRes;
        defaultRes.attackHappened = true;
        return defaultRes;
    }

    return hitEffectStrategy->apply(attacker, target, modifiers, *this, combatSystem);
}

Weapon::~Weapon() = default;
