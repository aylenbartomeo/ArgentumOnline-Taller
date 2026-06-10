#include "server/src/model/items/WeaponFactory.h"

#include <stdexcept>
#include <utility>

#include "server/src/model/interfaces/CombatStrategies.h"

WeaponFactory::WeaponFactory(std::unordered_map<std::string, WeaponConfig> configs):
        configs(std::move(configs)) {}

// --- Métodos estáticos (usados por tests y código legado) ---

std::unique_ptr<IAttackDelivery> WeaponFactory::createDeliveryStrategy(WeaponType type) {
    if (type == WeaponType::MELEE) {
        return std::make_unique<InstantMeleeDelivery>();
    } else if (type == WeaponType::MAGIC || type == WeaponType::RANGED) {
        return std::make_unique<ProjectileDelivery>();
    }
    return std::make_unique<InstantMeleeDelivery>();
}

std::unique_ptr<IHitEffect> WeaponFactory::createHitEffectStrategy(WeaponType type) {
    if (type == WeaponType::MELEE || type == WeaponType::RANGED) {
        return std::make_unique<MeleeDamageEffect>();
    } else if (type == WeaponType::MAGIC) {
        return std::make_unique<MagicDamageEffect>();
    }
    return std::make_unique<MeleeDamageEffect>();
}

// --- Métodos de instancia: priorizan el nombre explícito del TOML,
//     hacen fallback al tipo de arma si el campo está vacío. ---

std::unique_ptr<IAttackDelivery> WeaponFactory::createDelivery(const std::string& deliveryName,
                                                               WeaponType type) const {
    if (deliveryName == "instant_melee") {
        return std::make_unique<InstantMeleeDelivery>();
    } else if (deliveryName == "projectile") {
        return std::make_unique<ProjectileDelivery>();
    }
    // Fallback: inferir según WeaponType (campo delivery vacío o no reconocido)
    return createDeliveryStrategy(type);
}

std::unique_ptr<IHitEffect> WeaponFactory::createHitEffect(const std::string& hitEffectName,
                                                           WeaponType type) const {
    if (hitEffectName == "melee_damage") {
        return std::make_unique<MeleeDamageEffect>();
    } else if (hitEffectName == "magic_damage") {
        return std::make_unique<MagicDamageEffect>();
    } else if (hitEffectName == "magic_heal") {
        return std::make_unique<MagicHealEffect>();
    }
    // Fallback: inferir según WeaponType (campo hit_effect vacío o no reconocido)
    return createHitEffectStrategy(type);
}

std::unique_ptr<Weapon> WeaponFactory::create(const std::string& itemName) const {
    const auto config = configs.find(itemName);
    if (config == configs.end()) {
        throw std::invalid_argument("Unknown weapon item: " + itemName);
    }

    const WeaponConfig& itemConfig = config->second;
    return std::make_unique<Weapon>(itemConfig.id, itemName, itemConfig.price, itemConfig.type,
                                    itemConfig.minDamage, itemConfig.maxDamage,
                                    itemConfig.attackRange, itemConfig.manaCost,
                                    createDelivery(itemConfig.delivery, itemConfig.type),
                                    createHitEffect(itemConfig.hitEffect, itemConfig.type));
}
