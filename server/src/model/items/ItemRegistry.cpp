#include "server/src/model/items/ItemRegistry.h"

#include "server/src/config/ItemConfigLoader.h"
#include "server/src/model/items/BodyArmor.h"
#include "server/src/model/items/Helmet.h"
#include "server/src/model/items/Shield.h"

ItemRegistry::ItemRegistry(const std::filesystem::path& configPath) {
    auto weaponConfigs = ItemConfigLoader::loadWeaponConfigs(configPath);
    auto armorConfigs = ItemConfigLoader::loadArmorConfigs(configPath);

    for (const auto& [name, config]: weaponConfigs) {
        catalog[config.id] =
                std::make_unique<Weapon>(config.id, name, config.minDamage, config.maxDamage,
                                         config.type, config.attackRange, config.manaCost);
    }

    for (const auto& [name, config]: armorConfigs) {
        if (config.slot == ArmorSlot::Body) {
            catalog[config.id] = std::make_unique<BodyArmor>(config.id, name, config.minDefense,
                                                             config.maxDefense);
        } else if (config.slot == ArmorSlot::Head) {
            catalog[config.id] =
                    std::make_unique<Helmet>(config.id, name, config.minDefense, config.maxDefense);
        } else if (config.slot == ArmorSlot::Shield) {
            catalog[config.id] =
                    std::make_unique<Shield>(config.id, name, config.minDefense, config.maxDefense);
        }
    }
}

const Item* ItemRegistry::get_item(int item_id) const {
    auto it = catalog.find(item_id);
    if (it != catalog.end()) {
        return it->second.get();
    }
    return nullptr;
}

const Weapon* ItemRegistry::get_weapon(int item_id) const {
    return dynamic_cast<const Weapon*>(get_item(item_id));
}

const Armor* ItemRegistry::get_armor(int item_id) const {
    return dynamic_cast<const Armor*>(get_item(item_id));
}
