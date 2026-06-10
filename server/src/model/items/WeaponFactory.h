#ifndef SERVER_SRC_MODEL_ITEMS_WEAPONFACTORY_H
#define SERVER_SRC_MODEL_ITEMS_WEAPONFACTORY_H

#include <memory>
#include <string>
#include <unordered_map>

#include "server/src/model/items/Weapon.h"

class IAttackDelivery;
class IHitEffect;

struct WeaponConfig {
    int id;
    int price;
    WeaponType type;
    int minDamage;
    int maxDamage;
    int attackRange;
    int manaCost;
    std::string delivery;
    std::string hitEffect;
};

class WeaponFactory {
private:
    std::unordered_map<std::string, WeaponConfig> configs;

    std::unique_ptr<IAttackDelivery> createDelivery(const std::string& deliveryName, WeaponType type) const;
    std::unique_ptr<IHitEffect> createHitEffect(const std::string& hitEffectName, WeaponType type) const;

public:
    explicit WeaponFactory(std::unordered_map<std::string, WeaponConfig> configs);

    std::unique_ptr<Weapon> create(const std::string& itemName) const;

    static std::unique_ptr<IAttackDelivery> createDeliveryStrategy(WeaponType type);
    static std::unique_ptr<IHitEffect> createHitEffectStrategy(WeaponType type);
};

#endif
