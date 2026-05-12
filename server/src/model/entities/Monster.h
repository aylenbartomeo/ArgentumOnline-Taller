#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include <utility>

#include "../utils/position.h"
#include "../utils/types.h"
#include "interfaces/combatant.h"
#include "interfaces/interactable.h"
#include "server/src/config/MonsterConfig.h"

class Monster: public Combatant {
private:
    uint32_t id;
    NPCType type;
    std::string zone;  // bosques, mazmorras
    Position pos;

    int health;
    int max_health;

    int detection_range;
    int attack_range;
    int strength;
    int agility;
    int attack_min;
    int attack_max;


public:
    Monster(uint32_t id, NPCType type, Position pos, const MonsterConfig& config);

    void move(const Position& new_pos);
    void receive_damage(int amount) override;
    void attack(Combatant& target) override;
    bool is_dead() const override;
    Position get_position() const override;

    uint16_t get_strength() const override;
    int get_detection_range() const;
    int get_attack_range() const;
    const std::string& get_zone() const;
};

#endif
