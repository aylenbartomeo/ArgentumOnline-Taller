#include "AttackStrategy.h"

class RangedStrategy: public AttackStrategy {
public:
    bool execute(const Weapon& weapon, Combatant& attacker, Combatant& target) override {
        const int distance = attacker.get_position().distance_to(target.get_position());

        if (distance == 0 || distance > weapon.getAttackRange()) {
            return false;
        }

        uint16_t strength = attacker.get_strength();
        uint16_t damage = weapon.calculateDamage(strength);
        target.receive_damage(static_cast<int>(damage));

        return true;
    }
};
