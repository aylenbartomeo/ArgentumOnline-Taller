#include "server/src/model/items/Equipment.h"

Equipment::Equipment(): bodyArmor(nullptr), helmet(nullptr), shield(nullptr), weapon(nullptr) {}

void Equipment::equipBodyArmor(BodyArmor& armor) { this->bodyArmor = &armor; }

void Equipment::equipHelmet(Helmet& helmet) { this->helmet = &helmet; }

void Equipment::equipShield(Shield& shield) { this->shield = &shield; }

void Equipment::equipWeapon(Weapon& weapon) { this->weapon = &weapon; }

BodyArmor* Equipment::getBodyArmor() const { return this->bodyArmor; }

Helmet* Equipment::getHelmet() const { return this->helmet; }

Shield* Equipment::getShield() const { return this->shield; }

Weapon* Equipment::getWeapon() const { return this->weapon; }

int Equipment::getDefense() const {
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
