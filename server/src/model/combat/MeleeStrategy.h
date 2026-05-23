#include "AttackStrategy.h"

class MeleeStrategy : public AttackStrategy {
public:
    bool execute(const Weapon& weapon, Player& attacker, Attackable& target) override {};
};
