#include "EquipmentComponent.h"
#include "model/items/Item.h"
#include "model/items/Weapon.h"
#include "model/items/BodyArmor.h"
#include "model/items/Helmet.h"
#include "model/items/Shield.h"

EquipmentComponent::EquipmentComponent(): bodyArmor(nullptr), helmet(nullptr), shield(nullptr), weapon(nullptr) {}

uint32_t EquipmentComponent::equipItem(const Item* item) {
    if (!item)
        return 0;
    return item->equip_on(*this);
}

uint32_t EquipmentComponent::equipBodyArmor(const BodyArmor* armor) {
    uint32_t old_id =
            (this->bodyArmor != nullptr) ? static_cast<uint32_t>(this->bodyArmor->getId()) : 0;
    this->bodyArmor = armor;
    return old_id;
}

uint32_t EquipmentComponent::equipHelmet(const Helmet* helmet) {
    uint32_t old_id = (this->helmet != nullptr) ? static_cast<uint32_t>(this->helmet->getId()) : 0;
    this->helmet = helmet;
    return old_id;
}

uint32_t EquipmentComponent::equipShield(const Shield* shield) {
    uint32_t old_id = (this->shield != nullptr) ? static_cast<uint32_t>(this->shield->getId()) : 0;
    this->shield = shield;
    return old_id;
}

uint32_t EquipmentComponent::equipWeapon(const Weapon* new_weapon) {
    uint32_t old_id = (this->weapon != nullptr) ? static_cast<uint32_t>(this->weapon->getId()) : 0;
    this->weapon = new_weapon;
    return old_id;
}

int EquipmentComponent::getDefense() const {
    int defense = 0;

    if (this->bodyArmor != nullptr) {
        defense += this->bodyArmor->getDefense();
    }

    if (this->helmet != nullptr) {
        defense += this->helmet->getDefense();
    }

    if (this->shield != nullptr) {
        defense += this->shield->getDefense();
    }

    return defense;
}
