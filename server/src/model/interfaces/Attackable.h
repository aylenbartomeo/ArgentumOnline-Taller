#ifndef COMBATANT_H
#define COMBATANT_H

#include <cstdint>

#include "position.h"

class Attackable {
public:
    virtual ~Attackable() = default;

    // Consultas de estado
    virtual bool isDead() const = 0;
    virtual bool canBeAttacked() const = 0;
    virtual Position getPosition() const = 0;

    // Distancia Chebyshev a otro Attackable
    int distance_to(const Attackable& other) const {
        return getPosition().chebyshev_distance_to(other.getPosition());
    }

    // Atributos de combate
    virtual uint16_t getStrength() const = 0;
    virtual uint16_t getIntelligence() const = 0;
    virtual uint16_t getAgility() const = 0;
    virtual uint16_t getLevel() const = 0;
    virtual uint16_t getMaxHp() const = 0;
    virtual uint16_t getMana() const = 0;
    virtual int getDefense() const = 0;

    // Acciones que recibe
    virtual void receiveDamage(int amount) = 0;
    virtual bool consumeMana(int amount) = 0;
    virtual void handleDeath() = 0;

    // Fair play (Player lo restringe, Monster retorna true siempre)
    virtual bool canEngageInCombatWith(const Attackable& other) const = 0;
};

#endif
