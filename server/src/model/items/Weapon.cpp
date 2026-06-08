#include "Weapon.h"
#include <stdexcept>
#include <utility>

#include "../interfaces/CombatStrategies.h"
#include "../components/EquipmentComponent.h"
#include "FormulaEngine.h"

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
    
    // --- INYECCIÓN AUTOMÁTICA SEGÚN EL TIPO (COMPATIBILIDAD) ---
    if (type == WeaponType::MELEE) {
        deliveryStrategy = std::make_unique<InstantMeleeDelivery>();
        hitEffectStrategy = std::make_unique<MeleeDamageEffect>();
    } 
    else if (type == WeaponType::MAGIC) {
        // ¡BASTONES MÁGICOS! Disparan proyectil y consumen maná
        deliveryStrategy = std::make_unique<ProjectileDelivery>();
        hitEffectStrategy = std::make_unique<MagicDamageEffect>();
    }
    else if (type == WeaponType::RANGED) {
        // ¡ARCOS! Flechas infinitas (no consumen recurso), generan daño físico mediante proyectil
        deliveryStrategy = std::make_unique<ProjectileDelivery>();
        hitEffectStrategy = std::make_unique<MeleeDamageEffect>();
    }
    // else if (type == WeaponType::SUPPORT_MAGIC) {
    //     deliveryStrategy = std::make_unique<ProjectileDelivery>(); // Dispara un rayo/proyectil sanador
    //     hitEffectStrategy = std::make_unique<MagicHealEffect>();   // Que cura al impactar
    // }
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

Weapon::~Weapon() = default;