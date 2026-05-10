#include "AttackStrategy.h"

class MeleeStrategy : public AttackStrategy {
public:
    bool execute(const Weapon& weapon, Combatant& attacker, Combatant& target, 
        FormulaEngine& formulas) override {
        uint16_t strength = attacker.get_strength();

        uint16_t damage = formulas.calculate_base_damage(strength, weapon.getMinDamage(), weapon.getMaxDamage());

        target.receive_damage(static_cast<int>(damage));
                  
        return true;
    }
};