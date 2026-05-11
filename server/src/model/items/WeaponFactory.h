#ifndef SERVER_SRC_MODEL_ITEMS_WEAPONFACTORY_H
#define SERVER_SRC_MODEL_ITEMS_WEAPONFACTORY_H

#include <memory>
#include <string>
#include <unordered_map>

#include "server/src/model/items/Weapon.h"

struct WeaponConfig {
    int id;
    WeaponType type;
    int minDamage;
    int maxDamage;
    int attackRange;
    int manaCost;
};

class WeaponFactory {
private:
    std::unordered_map<std::string, WeaponConfig> configs;

public:
    explicit WeaponFactory(std::unordered_map<std::string, WeaponConfig> configs);

    std::unique_ptr<Weapon> create(const std::string& itemName) const;
};

#endif
