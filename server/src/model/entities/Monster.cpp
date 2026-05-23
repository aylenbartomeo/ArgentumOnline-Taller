#include "Monster.h"

#include <utility>

#include "../include/model/FormulaEngine.h"
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
    if (FormulaEngine::getInstance().is_attack_eluded(static_cast<uint16_t>(this->agility))) {
        return;
    }
    this->health -= amount;
    if (this->health < 0)
        this->health = 0;
}

bool Monster::isDead() const { return health <= 0; }

Position Monster::getPosition() const { return this->pos; }

void Monster::setPosition(const Position& newPos) { this->pos = newPos; }

uint16_t Monster::getStrength() const { return this->strength; }

int Monster::get_detection_range() const { return this->detection_range; }

int Monster::get_attack_range() const { return this->attack_range; }

const std::string& Monster::get_zone() const { return this->zone; }

uint16_t Monster::getAgility() const {
    // Podés retornar un valor base del monstruo o delegarlo a sus stats internas
    return agility; // O el atributo que use tu struct de NPC
}

uint16_t Monster::getLevel() const { return level; }
