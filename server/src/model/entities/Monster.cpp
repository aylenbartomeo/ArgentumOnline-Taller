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
        level(config.level) {}

void Monster::move(const Position& new_pos) {
    // Lógica de movimiento, podría ser aleatorio dentro de un rango o siguiendo al jugador si lo
    // detecta
    this->pos = new_pos;
}

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
        case NPCType::GOBLIN: return "Goblin";
        case NPCType::SKELETON: return "Skeleton";
        case NPCType::ZOMBIE: return "Zombie";
        case NPCType::SPIDER: return "Spider";
        case NPCType::ORC: return "Orc";
        case NPCType::GOLEM: return "Golem";
        default: return "Monster";
    }
}

int Monster::getAttackMin() const { return this->attack_min; }
int Monster::getAttackMax() const { return this->attack_max; }

uint16_t Monster::getAgility() const {
    // Podés retornar un valor base del monstruo o delegarlo a sus stats internas
    return agility;  // O el atributo que use tu struct de NPC
}

uint16_t Monster::getLevel() const { return level; }
uint16_t Monster::getMaxHp() const { return max_health; }
int Monster::getDefense() const { return 0; }    // Monsters could have base defense, but for now 0

bool Monster::canBeAttacked() const { return !isDead(); }
void Monster::handleDeath() {
    this->health = 0;  // State handled implicitly by isDead()
}
bool Monster::canEngageInCombatWith(const Attackable& /*other*/) const {
    return true;
}  // Monsters can attack anyone
