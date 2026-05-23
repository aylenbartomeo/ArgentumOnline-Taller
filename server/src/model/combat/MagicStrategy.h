#include "../interfaces/MagicUser.h"

#include "AttackStrategy.h"

class MagicStrategy : public AttackStrategy {
public:
    bool execute(const Weapon& weapon, Player& attacker, Attackable& target) override {};
};
