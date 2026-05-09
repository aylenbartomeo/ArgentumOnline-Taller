#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>
#include "combatant.h"
#include "../utils/position.h"
#include "../utils/types.h"
#include "interfaces/combatant.h"
#include "interfaces/interactable.h"

class Player : public Combatant, Interactable {
private:
    // Identidad y Posición
    uint32_t id;
    std::string name;
    Race race;
    CharacterClass char_class;
    Position pos;

    // --- Atributos de Rol (Influyen en fórmulas) ---
    int strength;      // Fuerza (Daño)
    int intelligence;  // Inteligencia (Maná)
    int agility;       // Agilidad (Esquivar)
    int constitution;  // Constitución (Vida)

    // --- Estados Dinámicos ---
    int health;
    int max_health;    // Calculado por Constitución + Clase + Nivel
    int mana;
    int max_mana;      // Calculado por Inteligencia + Clase + Nivel
    uint32_t gold;
    uint32_t max_gold; // Limitado por el Nivel
    
    uint32_t experience;
    uint16_t level;

    /** Metodos para usar con los NPCs ciudadanos */

    /* Comprar objetos */
    void buy(const std::vector<std::string> &params);

    /* Vender objetos */
    void sell(const std::vector<std::string> &params);

    /* Revivir*/
    void respawn();

    /* Sanar */
    void heal();

    /* Depositar objeto en el banco */
    void deposit_object(const std::vector<std::string> &params);

     /* Retirar objeto del banco */
    void withdraw_object(const std::vector<std::string> &params);   

    /* Despositar el oro en el banco */
    void deposit_gold(const std::vector<std::string> &params);

     /* Retirar oro del banco */
    void withdraw_gold(const std::vector<std::string> &params);

public:
    Player(uint32_t id, const std::string& name, Race race, CharacterClass char_class, 
           Position pos);

    /* Métodos de Combatant */
    void receive_damage(int amount) override;
    void attack(Combatant& target) override;
    bool is_dead() const override;

    /* Llamaria adentro a los metodos utilizados con los ciudadanos */
    void interact(Interactable& interactable, const std::string& action, const std::vector<std::string>& params);

};

#endif
