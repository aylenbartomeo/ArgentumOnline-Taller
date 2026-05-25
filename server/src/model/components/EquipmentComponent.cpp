#include "EquipmentComponent.h"

#include "model/FormulaEngine.h"
#include "model/items/BodyArmor.h"
#include "model/items/Helmet.h"
#include "model/items/Item.h"
#include "model/items/Shield.h"
#include "model/items/Weapon.h"

EquipmentComponent::EquipmentComponent():
        bodyArmor(nullptr), helmet(nullptr), shield(nullptr), weapon(nullptr) {}

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

uint16_t EquipmentComponent::calculateCurrentDefense() const {
    uint16_t armMin = this->bodyArmor ? this->bodyArmor->getMinDefense() : 0;
    uint16_t armMax = this->bodyArmor ? this->bodyArmor->getMaxDefense() : 0;

    uint16_t shMin = this->shield ? this->shield->getMinDefense() : 0;
    uint16_t shMax = this->shield ? this->shield->getMaxDefense() : 0;

    uint16_t helmMin = this->helmet ? this->helmet->getMinDefense() : 0;
    uint16_t helmMax = this->helmet ? this->helmet->getMaxDefense() : 0;

    return FormulaEngine::getInstance().calculate_defense(armMin, armMax, shMin, shMax, helmMin,
                                                          helmMax);
}
