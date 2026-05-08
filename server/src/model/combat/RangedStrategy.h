#include "AttackStrategy.h"

class RangedStrategy : public AttackStrategy {
public:
    bool execute(const Weapon& weapon) override {
        std::cout << "[RANGED] Verificando linea de vision y rango..." << std::endl;
                
        std::cout << "[RANGED] Disparando a distancia con " << weapon.getName() 
                  << " causando entre " << weapon.getMinDamage() << " y " << weapon.getMaxDamage() 
                  << " de dano." << std::endl;
                  
        return true;
    }
};