#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include <utility>

#include "../interfaces/Attackable.h"
#include "../interfaces/interactable.h"
#include "../utils/position.h"
#include "../utils/types.h"
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
    int getAttackMin() const;
    int getAttackMax() const;
    const std::string& get_zone() const;

    /* IMPLEMENTACION DE ATTACKABLE */
    std::string getName() const override;
    void receiveDamage(int amount) override;
    bool isDead() const override;
    Position getPosition() const override;
    bool canBeAttacked() const override;

    uint16_t getStrength() const override;
    uint16_t getIntelligence() const override;
    uint16_t getAgility() const override;
    uint16_t getLevel() const override;
    uint16_t getHp() const { return health; }
    uint16_t getMaxHp() const override;
    uint16_t getSpriteId() const { return 25; }  // TODO: read from config
    int getDefense() const override;

    void handleDeath() override;
    bool canEngageInCombatWith(const Attackable& other) const override;
};

#endif
