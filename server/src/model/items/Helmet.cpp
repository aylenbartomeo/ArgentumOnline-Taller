#include "server/src/model/items/Helmet.h"

#include <utility>

Helmet::Helmet(const int id, std::string name, const int minDefense, const int maxDefense):
        Armor(id, std::move(name), minDefense, maxDefense) {}

int Helmet::getDefense() const { return rollDefense(); }
