#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "server/src/model/items/Consumable.h"
// Datos de configuración leídos desde el TOML para un consumible.
struct ConsumableConfig {
    int id;
    int price;
    ConsumableType type;
    int durationMs;
    int effectValue;
};
// Fábrica de instancias Consumable a partir de su configuración.
class ConsumableFactory {
private:
    std::unordered_map<std::string, ConsumableConfig> configs;

public:
    explicit ConsumableFactory(std::unordered_map<std::string, ConsumableConfig> configs);
    std::unique_ptr<Consumable> create(const std::string& itemName) const;
};
