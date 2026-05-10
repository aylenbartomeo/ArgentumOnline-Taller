#include "AttackStrategy.h"
#include "../entities/interfaces/MagicUser.h"

class MagicStrategy : public AttackStrategy {
public:
    bool execute(const Weapon& weapon, Combatant& attacker, Combatant& target, 
        FormulaEngine& formulas) override {
        // Intentamos convertir al atacante en un usuario de magia
        MagicUser* caster = dynamic_cast<MagicUser*>(&attacker);

        if (!caster) { return false; }

        int mana_cost = weapon.getManaCost();

        if (caster->get_mana() < mana_cost) {
            std::cout << "[MAGIC] No se puede lanzar el hechizo. Mana insuficiente." << std::endl;
            return false; 
        }

        caster->consume_mana(mana_cost);

        uint16_t damage = formulas.calculate_base_damage(1, weapon.getMinDamage(), weapon.getMaxDamage());

        target.receive_damage(static_cast<int>(damage));
                  
        return true;
    }
};