#include "AttackStrategy.h"

class RangedStrategy : public AttackStrategy {
public:
    bool execute(const Weapon& weapon, Combatant& attacker, Combatant& target, 
        FormulaEngine& formulas) override {

        std::cout << "[RANGED] Verificando que el objetivo este a distancia (distancia > 1)..." << std::endl;
        // Aquí se debería verificar la distancia entre el atacante y el objetivo
        // Si la distancia no es válida, el ataque falla

        std::cout << "[RANGED] Calculando dano usando multiplicador de Agilidad del personaje..." << std::endl;
        // Para ataques a distancia, podríamos usar la agilidad en lugar de la fuerza para calcular el daño
        // Por simplicidad, usaremos la fuerza como en el ataque cuerpo a cuerpo

        uint16_t strength = attacker.get_strength();
        uint16_t damage = formulas.calculate_base_damage(strength, weapon.getMinDamage(), weapon.getMaxDamage());
        target.receive_damage(static_cast<int>(damage));    
                  
        return true;
    }
};