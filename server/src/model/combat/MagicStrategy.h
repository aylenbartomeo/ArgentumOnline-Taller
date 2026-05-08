#include "AttackStrategy.h"

class MagicStrategy : public AttackStrategy {
public:
    bool execute(const Weapon& weapon) override {
        std::cout << "[MAGIC] Intentando lanzar hechizo con " << weapon.getName() << "..." << std::endl;
        
        std::cout << "[MAGIC] Verificando si el personaje tiene al menos " << weapon.getManaCost() << " de mana." << std::endl;

        
        std::cout << "[MAGIC] Descontando " << weapon.getManaCost() << " puntos de mana al atacante." << std::endl;
        
        std::cout << "[MAGIC] Impactando al objetivo con dano magico entre " 
                  << weapon.getMinDamage() << " y " << weapon.getMaxDamage() << "." << std::endl;
                  
        return true;
    }
};