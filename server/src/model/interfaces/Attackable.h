#ifndef COMBATANT_H
#define COMBATANT_H

#include <cstdint>
#include "position.h"

class Attackable {
public:
    virtual ~Attackable() = default;

    // Contrato para recibir daño
    virtual void receiveDamage(int amount) = 0;

    // Contratos de consulta de estado
    virtual bool isDead() const = 0;

    // Contrato para posición actual para validar alcance y adyacencia.
    virtual Position getPosition() const = 0;

    // Agregar lo que se considere...
};

#endif
