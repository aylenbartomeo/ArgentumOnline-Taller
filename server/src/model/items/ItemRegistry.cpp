#include "server/src/model/items/ItemRegistry.h"

#include "server/src/config/ItemConfigLoader.h"
#include "server/src/model/items/BodyArmor.h"
#include "server/src/model/items/Helmet.h"
#include "server/src/model/items/Shield.h"

ItemRegistry::ItemRegistry(const std::filesystem::path& configPath) {
    auto weaponConfigs = ItemConfigLoader::loadWeaponConfigs(configPath);
    auto armorConfigs = ItemConfigLoader::loadArmorConfigs(configPath);
    auto consumableConfigs = ItemConfigLoader::loadConsumableConfigs(configPath);

    for (const auto& [name, config]: weaponConfigs) {
        weapons[config.id] =
                std::make_unique<Weapon>(config.id, name, config.price,
                                         config.type, config.minDamage, config.maxDamage,
                                         config.attackRange, config.manaCost);
    }

    for (const auto& [name, config]: armorConfigs) {
        if (config.slot == ArmorSlot::Body) {
            armors[config.id] = std::make_unique<BodyArmor>(config.id, name, config.price,
                                                            config.minDefense,
                                                            config.maxDefense);
        } else if (config.slot == ArmorSlot::Head) {
            armors[config.id] =
                    std::make_unique<Helmet>(config.id, name, config.price, config.minDefense, config.maxDefense);
        } else if (config.slot == ArmorSlot::Shield) {
            armors[config.id] =
                    std::make_unique<Shield>(config.id, name, config.price, config.minDefense, config.maxDefense);
        }
    }

    for (const auto& [name, config]: consumableConfigs) {
        consumables[config.id] =
                std::make_unique<Consumable>(config.id, name, config.price,
                                             config.type, config.durationMs, config.effectValue);
    }
}

const Item* ItemRegistry::get_item(int item_id) const {
    if (const Weapon* w = get_weapon(item_id))
        return w;
    if (const Armor* a = get_armor(item_id))
        return a;
    if (const Consumable* c = get_consumable(item_id))
        return c;
    auto it = items.find(item_id);
    if (it != items.end()) {
        return it->second.get();
    }
    return nullptr;
}

const Weapon* ItemRegistry::get_weapon(int item_id) const {
    auto it = weapons.find(item_id);
    if (it != weapons.end()) {
        return it->second.get();
    }
    return nullptr;
}

const Armor* ItemRegistry::get_armor(int item_id) const {
    auto it = armors.find(item_id);
    if (it != armors.end()) {
        return it->second.get();
    }
    return nullptr;
}

const Consumable* ItemRegistry::get_consumable(int item_id) const {
    auto it = consumables.find(item_id);
    if (it != consumables.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool ItemRegistry::isStackable(int item_id) const {
    if (get_weapon(item_id) != nullptr || get_armor(item_id) != nullptr) {
        return false;
    }
    return true;
}
