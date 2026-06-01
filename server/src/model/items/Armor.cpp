#include "server/src/model/items/Armor.h"

#include <random>
#include <stdexcept>
#include <utility>

Armor::Armor(const int id, std::string name, const int price, const int minDefense,
             const int maxDefense):
        Item(id, std::move(name), price), minDefense(minDefense), maxDefense(maxDefense) {}

int Armor::rollDefense() const {
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(minDefense, maxDefense);
    return distribution(generator);
}

int Armor::getMinDefense() const { return minDefense; }

int Armor::getMaxDefense() const { return maxDefense; }
