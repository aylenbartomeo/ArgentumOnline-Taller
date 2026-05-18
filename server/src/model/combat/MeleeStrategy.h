#include "AttackStrategy.h"

class MeleeStrategy: public AttackStrategy {
public:
    bool execute(const Weapon& weapon, Combatant& attacker, Combatant& target) override {
        uint16_t strength = attacker.getStrength();

        uint16_t damage = weapon.calculateDamage(strength);

        target.receiveDamage(static_cast<int>(damage));

        return true;
    }
};
