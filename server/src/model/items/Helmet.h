#ifndef SERVER_SRC_MODEL_ITEMS_HELMET_H
#define SERVER_SRC_MODEL_ITEMS_HELMET_H

#include <string>

#include "server/src/model/items/Armor.h"

class Helmet final: public Armor {
public:
    Helmet(int id, std::string name, int minDefense, int maxDefense);

    int getDefense() const override;
};

#endif
