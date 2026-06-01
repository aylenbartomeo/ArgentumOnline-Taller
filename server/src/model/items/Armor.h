#ifndef SERVER_SRC_MODEL_ITEMS_ARMOR_H
#define SERVER_SRC_MODEL_ITEMS_ARMOR_H

#include <string>

#include "server/src/model/items/Item.h"

class Armor: public Item {
protected:
    int minDefense;
    int maxDefense;

    Armor(int id, std::string name, int price, int minDefense, int maxDefense);

    int rollDefense() const;

public:
    virtual ~Armor() = default;

    Armor(const Armor&) = default;
    Armor& operator=(const Armor&) = default;
    Armor(Armor&&) noexcept = default;
    Armor& operator=(Armor&&) noexcept = default;

    int getMinDefense() const;
    int getMaxDefense() const;

    virtual int getDefense() const = 0;
    bool isMagic() const override { return false; }
    bool is_wearable() const override { return true; }
};

#endif
