#ifndef SERVER_SRC_MODEL_ITEMS_SHIELD_H
#define SERVER_SRC_MODEL_ITEMS_SHIELD_H

#include <string>

#include "server/src/model/items/Armor.h"

class EquipmentComponent;

class Shield final: public Armor {
public:
    Shield(int id, std::string name, int minDefense, int maxDefense, int price = 0);

    int getDefense() const override;

    uint32_t equip_on(EquipmentComponent& equipment) const override;
};

#endif
