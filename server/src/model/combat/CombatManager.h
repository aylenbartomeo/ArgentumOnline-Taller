#ifndef COMBAT_MANAGER_H
#define COMBAT_MANAGER_H

#include "../interfaces/Attackable.h"

#include "AttackParams.h"

class Player;
class Monster;

class CombatManager {
private:
    CombatManager() = default;

    // Lógica compartida de combate. Retorna el daño final infligido, o -1 si
    // el ataque no se concretó (fuera de rango, esquivado, target muerto).
    int resolveCombat(const Attackable& attacker, Attackable& target, const AttackParams& params);

public:
    CombatManager(const CombatManager&) = delete;
    CombatManager& operator=(const CombatManager&) = delete;

    static CombatManager& getInstance() {
        static CombatManager instance;
        return instance;
    }

    // Player ataca a cualquier entidad (Player o Monster)
    void processAttack(Player& attacker, Attackable& target);

    // Monster ataca a cualquier entidad (típicamente un Player)
    void processAttack(const Monster& attacker, Attackable& target);
};

#endif
