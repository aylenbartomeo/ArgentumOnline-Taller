#include "server/src/model/items/BodyArmor.h"

#include <utility>

BodyArmor::BodyArmor(const int id, std::string name, const int minDefense, const int maxDefense):
        Armor(id, std::move(name), minDefense, maxDefense) {}

int BodyArmor::getDefense() const {
    return rollDefense();
}
