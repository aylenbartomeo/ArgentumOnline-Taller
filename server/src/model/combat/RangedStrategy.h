#include "AttackStrategy.h"

class RangedStrategy: public AttackStrategy {
public:
    bool execute(const Weapon& weapon, Combatant& attacker, Combatant& target) override {
        const int distance = attacker.getPosition().distance_to(target.getPosition());

        if (distance == 0 || distance > weapon.getAttackRange()) {
            return false;
        }

        uint16_t strength = attacker.getStrength();
        uint16_t damage = weapon.calculateDamage(strength);
        target.receiveDamage(static_cast<int>(damage));

        return true;
    }
};
