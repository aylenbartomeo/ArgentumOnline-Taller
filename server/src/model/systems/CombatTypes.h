#ifndef COMBAT_TYPES_H
#define COMBAT_TYPES_H

#include <cstdint>

class Monster;

class ICombatEventCallback {
public:
    virtual void onMonsterDeath(const Monster& monster, uint32_t killerDbId) = 0;
    virtual void onPlayerDeath(uint32_t dbId) = 0;
    virtual ~ICombatEventCallback() = default;
};

struct CombatResult {
    bool attackHappened = false;
    bool evaded = false;
    bool critical = false;
    int damage = 0;
    bool isPending = false;
};

struct AttackParams {
    uint16_t minDamage;
    uint16_t maxDamage;
    int attackRange;
    int manaCost;
    bool isMagic;
    float attackBonus = 1.0f;
    float defenseBonus = 1.0f;
};

#endif // COMBAT_TYPES_H
