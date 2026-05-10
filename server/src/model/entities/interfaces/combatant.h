// server/src/model/interfaces/combatant.h
#ifndef COMBATANT_H
#define COMBATANT_H

class Combatant {
public:
    virtual ~Combatant() = default;

    // Contrato para recibir daño
    virtual void receive_damage(int amount) = 0;

    // Contrato para atacar a otro
    virtual void attack(Combatant& target) = 0;

    // Método de consulta de estado
    virtual bool is_dead() const = 0;
};

#endif
