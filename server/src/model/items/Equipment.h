#ifndef SERVER_SRC_MODEL_ITEMS_EQUIPMENT_H
#define SERVER_SRC_MODEL_ITEMS_EQUIPMENT_H

#include "server/src/model/items/BodyArmor.h"
#include "server/src/model/items/Helmet.h"
#include "server/src/model/items/Shield.h"
#include "server/src/model/items/Weapon.h"

class Equipment {
private:
    const BodyArmor* bodyArmor;
    const Helmet* helmet;
    const Shield* shield;
    const Weapon* weapon;

public:
    Equipment();

    uint32_t equip_item(const Item* item);

    uint32_t equip_body_armor(const BodyArmor* armor);
    uint32_t equip_helmet(const Helmet* helmet);
    uint32_t equip_shield(const Shield* shield);
    uint32_t equip_weapon(const Weapon* weapon);

    const BodyArmor* getBodyArmor() const;
    const Helmet* getHelmet() const;
    const Shield* getShield() const;
    const Weapon* getWeapon() const;

    int getDefense() const;
};

#endif
