#include "server/src/model/items/ArmorFactory.h"

#include <stdexcept>
#include <utility>

#include "server/src/model/items/BodyArmor.h"
#include "server/src/model/items/Helmet.h"
#include "server/src/model/items/Shield.h"

ArmorFactory::ArmorFactory(std::unordered_map<std::string, ArmorConfig> configs):
        configs(std::move(configs)) {}

std::unique_ptr<Armor> ArmorFactory::create(const std::string& itemName) const {
    const auto config = configs.find(itemName);
    if (config == configs.end()) {
        throw std::invalid_argument("Unknown armor item: " + itemName);
    }

    const ArmorConfig& itemConfig = config->second;

    switch (itemConfig.slot) {
        case ArmorSlot::Body:
            return std::make_unique<BodyArmor>(itemConfig.id, itemName, itemConfig.minDefense,
                                               itemConfig.maxDefense);
        case ArmorSlot::Head:
            return std::make_unique<Helmet>(itemConfig.id, itemName, itemConfig.minDefense,
                                            itemConfig.maxDefense);
        case ArmorSlot::Shield:
            return std::make_unique<::Shield>(itemConfig.id, itemName, itemConfig.minDefense,
                                              itemConfig.maxDefense);
    }

    throw std::invalid_argument("Invalid armor slot for item: " + itemName);
}
