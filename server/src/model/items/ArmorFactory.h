#ifndef SERVER_SRC_MODEL_ITEMS_ARMORFACTORY_H
#define SERVER_SRC_MODEL_ITEMS_ARMORFACTORY_H

#include <memory>
#include <string>
#include <unordered_map>

#include "server/src/model/items/Armor.h"

enum class ArmorSlot {
    Body,
    Head,
    Shield,
};

struct ArmorConfig {
    int id;
    ArmorSlot slot;
    int minDefense;
    int maxDefense;
};

class ArmorFactory {
private:
    std::unordered_map<std::string, ArmorConfig> configs;

public:
    explicit ArmorFactory(std::unordered_map<std::string, ArmorConfig> configs);

    std::unique_ptr<Armor> create(const std::string& itemName) const;
};

#endif
