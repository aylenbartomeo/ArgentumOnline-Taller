#ifndef SERVER_SRC_MODEL_ITEMS_EQUIPMENT_H
#define SERVER_SRC_MODEL_ITEMS_EQUIPMENT_H

#include "server/src/model/items/BodyArmor.h"
#include "server/src/model/items/Helmet.h"
#include "server/src/model/items/Shield.h"
#include "server/src/model/items/Weapon.h"

class Equipment {
private:
    BodyArmor* bodyArmor;
    Helmet* helmet;
    Shield* shield;
    Weapon* weapon;

public:
    Equipment();

    void equipBodyArmor(BodyArmor& armor);
    void equipHelmet(Helmet& helmet);
    void equipShield(Shield& shield);
    void equipWeapon(Weapon& weapon);

    BodyArmor* getBodyArmor() const;
    Helmet* getHelmet() const;
    Shield* getShield() const;
    Weapon* getWeapon() const;

    int getDefense() const;
};

#endif
