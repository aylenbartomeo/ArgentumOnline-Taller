#ifndef SERVER_SRC_MODEL_ITEMS_SHIELD_H
#define SERVER_SRC_MODEL_ITEMS_SHIELD_H

#include <string>

#include "server/src/model/items/Armor.h"

class EquipmentComponent;

class Shield final: public Armor {
public:
    Shield(int id, std::string name, int price, int minDefense, int maxDefense);

    int getDefense() const override;

    void equip_on(EquipmentComponent& equipment, uint8_t slotIndex) const override;
};

#endif
