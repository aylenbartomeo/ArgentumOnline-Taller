#include "EquipmentComponent.h"

#include "model/items/BodyArmor.h"
#include "model/items/Helmet.h"
#include "model/items/Item.h"
#include "model/items/Shield.h"
#include "model/items/Weapon.h"

#include "FormulaEngine.h"

EquipmentComponent::EquipmentComponent():
        bodyArmor(nullptr), helmet(nullptr), shield(nullptr), weapon(nullptr) {}

bool EquipmentComponent::equipItem(const Item* item, uint8_t slotIndex) {
    if (!item)
        return false;
    item->equip_on(*this, slotIndex);
    return true;
}

void EquipmentComponent::equipBodyArmor(const BodyArmor* armor, uint8_t slotIndex) {
    this->bodyArmor = armor;
    this->bodyArmorSlot = slotIndex;
}

void EquipmentComponent::equipHelmet(const Helmet* helmet, uint8_t slotIndex) {
    this->helmet = helmet;
    this->helmetSlot = slotIndex;
}

void EquipmentComponent::equipShield(const Shield* shield, uint8_t slotIndex) {
    this->shield = shield;
    this->shieldSlot = slotIndex;
}

void EquipmentComponent::equipWeapon(const Weapon* new_weapon, uint8_t slotIndex) {
    this->weapon = new_weapon;
    this->weaponSlot = slotIndex;
}

void EquipmentComponent::unequipSlot(uint8_t slotIndex) {
    if (bodyArmorSlot.has_value() && bodyArmorSlot.value() == slotIndex)
        unequip_body_armor();
    if (helmetSlot.has_value() && helmetSlot.value() == slotIndex)
        unequip_helmet();
    if (shieldSlot.has_value() && shieldSlot.value() == slotIndex)
        unequip_shield();
    if (weaponSlot.has_value() && weaponSlot.value() == slotIndex)
        unequip_weapon();
}

bool EquipmentComponent::isSlotEquipped(uint8_t slotIndex) const {
    return (bodyArmorSlot.has_value() && bodyArmorSlot.value() == slotIndex) ||
           (helmetSlot.has_value() && helmetSlot.value() == slotIndex) ||
           (shieldSlot.has_value() && shieldSlot.value() == slotIndex) ||
           (weaponSlot.has_value() && weaponSlot.value() == slotIndex);
}

void EquipmentComponent::unequip_body_armor() {
    this->bodyArmor = nullptr;
    this->bodyArmorSlot = std::nullopt;
}

void EquipmentComponent::unequip_helmet() {
    this->helmet = nullptr;
    this->helmetSlot = std::nullopt;
}

void EquipmentComponent::unequip_shield() {
    this->shield = nullptr;
    this->shieldSlot = std::nullopt;
}

void EquipmentComponent::unequip_weapon() {
    this->weapon = nullptr;
    this->weaponSlot = std::nullopt;
}

uint16_t EquipmentComponent::calculateCurrentDefense() const {
    uint16_t armMin = this->bodyArmor ? this->bodyArmor->getMinDefense() : 0;
    uint16_t armMax = this->bodyArmor ? this->bodyArmor->getMaxDefense() : 0;

    uint16_t shMin = this->shield ? this->shield->getMinDefense() : 0;
    uint16_t shMax = this->shield ? this->shield->getMaxDefense() : 0;

    uint16_t helmMin = this->helmet ? this->helmet->getMinDefense() : 0;
    uint16_t helmMax = this->helmet ? this->helmet->getMaxDefense() : 0;

    return FormulaEngine::getInstance().calculateDefense(armMin, armMax, shMin, shMax, helmMin,
                                                          helmMax);
}
