#ifndef STATS_COMPONENT_H
#define STATS_COMPONENT_H

#include <cstdint>
#include <algorithm>

class StatsComponent {
private:
    // Atributos principales (Definen el escalado del personaje)
    uint8_t strength;      // Fuerza
    uint8_t intelligence;  // Inteligencia
    uint8_t agility;       // Agilidad
    uint8_t constitution;  // Constitución

    // Estadísticas dinámicas
    uint16_t health;
    uint16_t max_health;
    uint16_t mana;
    uint16_t max_mana;

    // Progresión
    uint32_t exp;
    uint16_t level;

public:
    // Constructor completo basado en tu nueva lista de atributos
    StatsComponent(uint8_t str, uint8_t intel, uint8_t agi, uint8_t consti,
                   uint16_t max_hp, uint16_t max_mp)
        : strength(str), intelligence(intel), agility(agi), constitution(consti),
          health(max_hp), max_health(max_hp),
          mana(max_mp), max_mana(max_mp),
          exp(0), level(1) {}

    // --- GETTERS ---
    uint8_t getStrength() const { return strength; }
    uint8_t getIntelligence() const { return intelligence; }
    uint8_t getAgility() const { return agility; }
    uint8_t getConstitution() const { return constitution; }

    uint16_t getHp() const { return health; }
    uint16_t getMaxHp() const { return max_health; }
    uint16_t getMana() const { return mana; }
    uint16_t getMaxMana() const { return max_mana; }
    uint32_t getExp() const { return exp; }
    uint16_t getLevel() const { return level; }

    // Manejo de Experiencia simplificado
    void addExperience(uint32_t amount) {
        // En un futuro, acá se puede chequear si 'exp' supera el umbral para disparar un levelUp
        exp += amount;
    }

    void takeDamage(uint16_t amount) {
        health = (amount >= health) ? 0 : health - amount;
    }

    void heal(uint16_t amount) {
        if (health == 0) return;
        health = std::min(static_cast<uint16_t>(health + amount), max_health);
    }
};

#endif
