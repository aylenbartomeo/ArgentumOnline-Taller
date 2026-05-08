#include "AttackStrategy.h"

class MeleeStrategy : public AttackStrategy {
public:
    bool execute(const Weapon& weapon) override {
        std::cout << "[MELEE] Verificando que el objetivo este al lado (distancia == 1)..." << std::endl;
        
        std::cout << "[MELEE] Calculando dano usando multiplicador de Fuerza del personaje..." << std::endl;
        
        std::cout << "[MELEE] Aplicando un dano aleatorio entre " 
                  << weapon.getMinDamage() << " y " << weapon.getMaxDamage() 
                  << " al objetivo con el arma " << weapon.getName() << "." << std::endl;
                  
        return true;
    }
};