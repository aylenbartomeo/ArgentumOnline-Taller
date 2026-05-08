#include "server/src/model/items/Armor.h"

#include <random>
#include <stdexcept>
#include <utility>

Armor::Armor(const int id, std::string name, const int minDefense, const int maxDefense):
        id(id), name(std::move(name)), minDefense(minDefense), maxDefense(maxDefense) {
    if (id < 0) {
        throw std::invalid_argument("Armor id cannot be negative");
    }

    if (this->name.empty()) {
        throw std::invalid_argument("Armor name cannot be empty");
    }

    if (minDefense < 0) {
        throw std::invalid_argument("Armor minimum defense cannot be negative");
    }

    if (minDefense > maxDefense) {
        throw std::invalid_argument("Armor minimum defense cannot exceed maximum defense");
    }
}

int Armor::rollDefense() const {
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(minDefense, maxDefense);
    return distribution(generator);
}

int Armor::getId() const {
    return id;
}

const std::string& Armor::getName() const {
    return name;
}

int Armor::getMinDefense() const {
    return minDefense;
}

int Armor::getMaxDefense() const {
    return maxDefense;
}
