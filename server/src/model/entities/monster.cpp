#include "monster.h"

Monster::Monster(uint32_t id, NPCType type, std::string zone, Position pos)
{
}

void Monster::move(const Position &new_pos)
{
    // Lógica de movimiento, podría ser aleatorio dentro de un rango o siguiendo al jugador si lo detecta
    this->pos = new_pos;
}

void Monster::receive_damage(int amount)
{
    // Lógica de mitigación por agilidad/armadura segun clase y equipo
    this->health -= amount;
}

void Monster::attack(Combatant& target) {
    // Calcular daño según fuerza y arma
    int damage; // Aca habria que agregar la logica para calcular el daño segun la clase, el arma equipada, etc. 
                // Con FormulaEngine
    target.receive_damage(damage);
}

bool Monster::is_dead() const { return health <= 0; }