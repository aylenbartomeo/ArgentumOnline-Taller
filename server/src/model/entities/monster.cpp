#include "monster.h"

#include <utility>
#include <FormulaEngine.h>
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
        attack_max(config.attackMax) {}

void Monster::move(const Position& new_pos) {
    // Lógica de movimiento, podría ser aleatorio dentro de un rango o siguiendo al jugador si lo
    // detecta
    this->pos = new_pos;
}

void Monster::receive_damage(int amount) {
    if (FormulaEngine::getInstance().is_attack_eluded(static_cast<uint16_t>(this->agility))) {
        return;
    }
    this->health -= amount;
    if (this->health < 0)
        this->health = 0;
}

void Monster::attack(Combatant& target) {
    const int distance = this->pos.distance_to(target.get_position());
    if (distance == 0 || distance > this->attack_range) {
        return;
    }

    uint16_t damage = FormulaEngine::getInstance().calculate_base_damage(static_cast<uint16_t>(this->strength),
                                                           static_cast<uint16_t>(this->attack_min),
                                                           static_cast<uint16_t>(this->attack_max));
    target.receive_damage(static_cast<int>(damage));
}

bool Monster::is_dead() const { return health <= 0; }

Position Monster::get_position() const { return this->pos; }

uint16_t Monster::get_strength() const { return this->strength; }

int Monster::get_detection_range() const { return this->detection_range; }

int Monster::get_attack_range() const { return this->attack_range; }

const std::string& Monster::get_zone() const { return this->zone; }
