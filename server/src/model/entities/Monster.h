#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include <utility>

#include "../utils/position.h"
#include "../utils/types.h"
#include "../interfaces/Attackable.h"
#include "../interfaces/interactable.h"
#include "server/src/config/MonsterConfig.h"

class Monster: public Attackable {
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
    int level;

public:
    Monster(uint32_t id, NPCType type, Position pos, const MonsterConfig& config);

    void move(const Position& new_pos);
    void setPosition(const Position& newPos);
    int get_detection_range() const;
    int get_attack_range() const;
    const std::string& get_zone() const;
    
    /* IMPLEMENTACION DE COMBATANT */
    void receiveDamage(int amount) override;
    bool isDead() const override;
    Position getPosition() const override;
    uint16_t getStrength() const;
    uint16_t getAgility() const;
    uint16_t getLevel() const;
};

#endif
