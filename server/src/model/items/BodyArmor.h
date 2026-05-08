#ifndef SERVER_SRC_MODEL_ITEMS_BODYARMOR_H
#define SERVER_SRC_MODEL_ITEMS_BODYARMOR_H

#include "server/src/model/items/Armor.h"

#include <string>

class BodyArmor final: public Armor {
public:
    BodyArmor(int id, std::string name, int minDefense, int maxDefense);

    int getDefense() const override;
};

#endif
