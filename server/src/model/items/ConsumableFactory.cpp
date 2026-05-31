#include "ConsumableFactory.h"

#include <stdexcept>
#include <utility>

ConsumableFactory::ConsumableFactory(
        std::unordered_map<std::string, ConsumableConfig> configs):
        configs(std::move(configs)) {}

std::unique_ptr<Consumable> ConsumableFactory::create(const std::string& itemName) const {
    const auto it = configs.find(itemName);
    if (it == configs.end()) {
        throw std::invalid_argument("Unknown consumable item: " + itemName);
    }

    const ConsumableConfig& cfg = it->second;
    return std::make_unique<Consumable>(cfg.id, itemName, cfg.price,
                                       cfg.type, cfg.durationMs, cfg.effectValue);
}
