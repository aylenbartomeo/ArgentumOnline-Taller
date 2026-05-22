#ifndef COMBATANT_H
#define COMBATANT_H

#include <cstdint>
#include "position.h"

#include "../../../../../common/utils/position.h"

class Combatant {
public:
    virtual ~Combatant() = default;

    // Contrato para recibir daño
    virtual void receiveDamage(int amount) = 0;

    // Contrato para atacar a otro
    virtual void attack(Combatant& target) = 0;

    // Método de consulta de estado
    virtual bool isDead() const = 0;

    // Posición actual para validar alcance y adyacencia.
    virtual Position getPosition() const = 0;

    // Métodos para el combate físico
    virtual uint16_t getStrength() const = 0;

    virtual uint16_t getAgility() const = 0;

    virtual uint16_t getTotalDefense() const = 0;
};

#endif
