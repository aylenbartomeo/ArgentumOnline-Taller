#ifndef SERVER_SRC_MODEL_ITEMS_SHIELD_H
#define SERVER_SRC_MODEL_ITEMS_SHIELD_H

#include "server/src/model/items/Armor.h"

#include <string>

class Shield final: public Armor {
public:
    Shield(int id, std::string name, int minDefense, int maxDefense);

    int getDefense() const override;
};

#endif
