#ifndef SERVER_SRC_MODEL_ITEMS_BODYARMOR_H
#define SERVER_SRC_MODEL_ITEMS_BODYARMOR_H

#include <string>

#include "Armor.h"

class EquipmentComponent;

class BodyArmor final: public Armor {
public:
    BodyArmor(int id, std::string name, int minDefense, int maxDefense, int price = 0);

    int getDefense() const override;

    void equip_on(EquipmentComponent& equipment, uint8_t slotIndex) const override;
};

#endif
