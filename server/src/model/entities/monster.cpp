#include "monster.h"

Monster::Monster(uint32_t id, NPCType type, std::string zone, Position pos, FormulaEngine& formulas)
    : id(id), type(type), zone(zone), pos(pos), formulas(formulas)
{
}

void Monster::move(const Position &new_pos)
{
    // Lógica de movimiento, podría ser aleatorio dentro de un rango o siguiendo al jugador si lo detecta
    this->pos = new_pos;
}

void Monster::receive_damage(int amount) {
    // Los monstruos también pueden esquivar si tienen agilidad, se usaría this->agility
    if (this->formulas.is_attack_eluded(static_cast<uint16_t>(10))) { 
        return; 
    }
    this->health -= amount;
    if (this->health < 0) this->health = 0;
}

void Monster::attack(Combatant& target) {
    // VALORES DE PRUEBA (Luego vendrán del TOML)
    uint16_t monster_str = 10;
    uint16_t m_min = 2;
    uint16_t m_max = 6;

    uint16_t damage = this->formulas.calculate_base_damage(monster_str, m_min, m_max);
    target.receive_damage(static_cast<int>(damage));
}

bool Monster::is_dead() const { return health <= 0; }

uint16_t Monster::get_strength() const { return this->strength; }