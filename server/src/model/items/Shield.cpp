#include "server/src/model/items/Shield.h"

#include <utility>

Shield::Shield(const int id, std::string name, const int minDefense, const int maxDefense):
        Armor(id, std::move(name), minDefense, maxDefense) {}

int Shield::getDefense() const { return rollDefense(); }
