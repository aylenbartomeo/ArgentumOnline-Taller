#include "Monster.h"

#include <utility>

Monster::Monster(uint32_t id, NPCType type, Position pos, const MonsterConfig& config):
        id(id),
        type(type),
        zone(config.zone),
        pos(pos),
        health(config.maxHealth),
        max_health(config.maxHealth),
        detection_range(config.detectionRange),
        attack_range(config.attackRange),
        strength(config.strength),
        agility(config.agility),
        attack_min(config.attackMin),
        attack_max(config.attackMax),
        level(config.level),
        attack_cooldown_ms(config.attackCooldownMs),
        move_cooldown_ms(config.moveCooldownMs),
        time_since_last_attack(0.0f),
        time_since_last_move(0.0f) {}

void Monster::move(const Position& new_pos) { this->pos = new_pos; }

void Monster::update(float deltaMs) {
    time_since_last_attack += deltaMs;
    time_since_last_move += deltaMs;
    if (actionTimerMs > 0.0f) {
        actionTimerMs -= deltaMs;
        if (actionTimerMs <= 0.0f) {
            currentAction = 0;
            actionTimerMs = 0.0f;
        }
    }
}

bool Monster::canAttack() const { return time_since_last_attack >= attack_cooldown_ms; }

bool Monster::canMove() const { return time_since_last_move >= move_cooldown_ms; }

void Monster::resetAttackCooldown() { time_since_last_attack = 0.0f; }

void Monster::resetMoveCooldown() { time_since_last_move = 0.0f; }

void Monster::receiveDamage(int amount) {
    if (amount < 0)
        return;
    this->health -= amount;
    if (this->health < 0)
        this->health = 0;
}

bool Monster::isDead() const { return health <= 0; }

Position Monster::getPosition() const { return this->pos; }

void Monster::setPosition(const Position& newPos) { this->pos = newPos; }

uint16_t Monster::getStrength() const { return this->strength; }
uint16_t Monster::getIntelligence() const { return 0; }  // Monsters don't use magic yet

int Monster::get_detection_range() const { return this->detection_range; }

int Monster::get_attack_range() const { return this->attack_range; }

const std::string& Monster::get_zone() const { return this->zone; }

std::string Monster::getName() const {
    switch (this->type) {
        case NPCType::GOBLIN:
            return "Goblin";
        case NPCType::SKELETON:
            return "Skeleton";
        case NPCType::ZOMBIE:
            return "Zombie";
        case NPCType::SPIDER:
            return "Spider";
        case NPCType::ORC:
            return "Orc";
        case NPCType::GOLEM:
            return "Golem";
        default:
            return "Monster";
    }
}

EntityDTO Monster::toEntityDTO() const {
    EntityDTO dto;
    dto.id = id;
    dto.type = EntityType::MONSTER;
    dto.x = pos.x;
    dto.y = pos.y;
    dto.current_hp = static_cast<uint16_t>(health);
    dto.max_hp = static_cast<uint16_t>(max_health);
    dto.entityTypeId = static_cast<uint8_t>(type);
    dto.action = currentAction;
    return dto;
}

void Monster::setAction(uint8_t action, float durationMs) {
    currentAction = action;
    actionTimerMs = durationMs;
}

int Monster::getAttackMin() const { return this->attack_min; }
int Monster::getAttackMax() const { return this->attack_max; }

uint16_t Monster::getAgility() const {
    // Podés retornar un valor base del monstruo o delegarlo a sus stats internas
    return agility;  // O el atributo que use tu struct de NPC
}

uint16_t Monster::getLevel() const { return level; }
uint16_t Monster::getMaxHp() const { return max_health; }
int Monster::getDefense() const { return 0; }  // Monsters could have base defense, but for now 0

bool Monster::canBeAttacked() const { return !isDead(); }
void Monster::handleDeath() {
    this->health = 0;  // State handled implicitly by isDead()
}
bool Monster::canEngageInCombatWith(const Attackable& /*other*/) const {
    return true;
}  // Monsters can attack anyone

MonsterPersistData Monster::toPersistData() const {
    MonsterPersistData d{};
    d.entityId = this->id;
    d.type = static_cast<uint8_t>(this->type);
    d.posX = this->pos.x;
    d.posY = this->pos.y;
    d.hp = static_cast<int16_t>(this->health);
    d.maxHp = static_cast<int16_t>(this->max_health);
    return d;
}

void Monster::fromPersistData(const MonsterPersistData& data) {
    this->pos.x = data.posX;
    this->pos.y = data.posY;
    this->health = data.hp;
    this->max_health = data.maxHp;
}
