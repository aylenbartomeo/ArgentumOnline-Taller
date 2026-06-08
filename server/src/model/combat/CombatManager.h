#ifndef COMBAT_MANAGER_H
#define COMBAT_MANAGER_H

#include "../interfaces/Attackable.h"

#include "AttackParams.h"

class Player;
class Monster;

struct CombatResult {
    bool attackHappened = false;
    bool evaded = false;
    bool critical = false;
    int damage = 0;
};

class CombatManager {
private:
    CombatManager() = default;

    // Lógica compartida de combate. Retorna el daño final infligido, o -1 si
    // el ataque no se concretó (fuera de rango, esquivado, target muerto).
    CombatResult resolveCombat(const Attackable& attacker, Attackable& target,
                               const AttackParams& params);

public:
    CombatManager(const CombatManager&) = delete;
    CombatManager& operator=(const CombatManager&) = delete;

    static CombatManager& getInstance() {
        static CombatManager instance;
        return instance;
    }

    CombatResult processProjectileAttack(Player& attacker, Attackable& target, uint16_t minDmg,
                                         uint16_t maxDmg, float attackBonus, float defenseBonus);

    // Player ataca a cualquier entidad (Player o Monster)
    CombatResult processAttack(Player& attacker, Attackable& target);

    // Monster ataca a cualquier entidad (típicamente un Player)
    CombatResult processAttack(const Monster& attacker, Attackable& target);

    // Variante con bonuses de clan pre-calculados por World
    CombatResult processAttack(Player& attacker, Attackable& target, float attackBonus,
                               float defenseBonus);
};

#endif
