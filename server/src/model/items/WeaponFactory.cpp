#include "server/src/model/items/WeaponFactory.h"

#include <stdexcept>
#include <utility>

WeaponFactory::WeaponFactory(std::unordered_map<std::string, WeaponConfig> configs):
        configs(std::move(configs)) {}

std::unique_ptr<Weapon> WeaponFactory::create(const std::string& itemName) const {
    const auto config = configs.find(itemName);
    if (config == configs.end()) {
        throw std::invalid_argument("Unknown weapon item: " + itemName);
    }

    const WeaponConfig& itemConfig = config->second;
    return std::make_unique<Weapon>(itemConfig.id, itemName, itemConfig.minDamage,
                                    itemConfig.maxDamage, itemConfig.type,
                                    itemConfig.attackRange, itemConfig.manaCost);
}
