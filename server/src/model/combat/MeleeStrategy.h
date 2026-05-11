#include "AttackStrategy.h"

class MeleeStrategy: public AttackStrategy {
public:
    bool execute(const Weapon& weapon, Combatant& attacker, Combatant& target,
                 FormulaEngine& formulas) override {
        uint16_t strength = attacker.get_strength();

        uint16_t damage = weapon.calculateDamage(strength, formulas);

        target.receive_damage(static_cast<int>(damage));

        return true;
    }
};
