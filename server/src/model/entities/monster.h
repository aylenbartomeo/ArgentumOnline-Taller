#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include "../utils/position.h"
#include "../utils/types.h"
#include "interfaces/combatant.h"

class Monster : public Combatant {
private:
    uint32_t id;
    NPCType type;
    std::string zone; //bosques, mazmorras
    Position pos;
    
    int health;
    int max_health;

    int detection_range;
    int attack_range;

public:
    Monster(uint32_t id, NPCType type, std::string zone, Position pos);

    void move(const Position& new_pos);
    void receive_damage(int amount) override;
    void attack(Combatant& target) override;
    bool is_dead() const override;
    
};

#endif
