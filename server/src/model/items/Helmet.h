#ifndef SERVER_SRC_MODEL_ITEMS_HELMET_H
#define SERVER_SRC_MODEL_ITEMS_HELMET_H

#include <string>

#include "server/src/model/items/Armor.h"

class EquipmentComponent;

class Helmet final: public Armor {
public:
    Helmet(int id, std::string name, int price, int minDefense, int maxDefense);

    int getDefense() const override;

    void equip_on(EquipmentComponent& equipment, uint8_t slotIndex) const override;
};

#endif
