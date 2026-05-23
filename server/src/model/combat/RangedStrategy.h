#include "AttackStrategy.h"

class RangedStrategy: public AttackStrategy {
public:
    bool execute(const Weapon& weapon, Player& attacker, Attackable& target) override {};
};
