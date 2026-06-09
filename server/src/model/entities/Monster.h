#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include <utility>

#include "../../common/include/dto/Snapshot.h"
#include "../../persistence/WorldPersistData.h"
#include "../interfaces/Attackable.h"
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

    float attack_cooldown_ms;
    float move_cooldown_ms;
    float time_since_last_attack;
    float time_since_last_move;
    uint32_t current_target_id = 0;
    uint8_t currentAction = 0;
    float actionTimerMs = 0.0f;

public:
    Monster(uint32_t id, NPCType type, Position pos, const MonsterConfig& config);

    void move(const Position& new_pos);
    void setPosition(const Position& newPos);
    int get_detection_range() const;
    int get_attack_range() const;
    int getAttackMin() const;
    int getAttackMax() const;
    const std::string& get_zone() const;

    void update(float deltaMs);
    bool canAttack() const;
    bool canMove() const;
    void resetAttackCooldown();
    void resetMoveCooldown();

    uint32_t getId() const { return id; }
    void setHealth(int hp) { this->health = hp; }
    MonsterPersistData toPersistData() const;
    void fromPersistData(const MonsterPersistData& data);

    uint32_t getTargetId() const { return current_target_id; }
    void setTargetId(uint32_t targetId) { current_target_id = targetId; }

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
    int getDefense() const override;

    EntityDTO toEntityDTO() const;
    void setAction(uint8_t action, float durationMs);
    uint8_t getCurrentAction() const { return currentAction; }
    NPCType getType() const { return type; }

    void handleDeath() override;
    bool canEngageInCombatWith(const Attackable& other) const override;
};

#endif
