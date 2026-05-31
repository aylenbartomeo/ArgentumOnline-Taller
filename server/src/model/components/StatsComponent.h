#pragma once
#include <algorithm>
#include <cstdint>

#include "../../include/model/FormulaEngine.h"
#include "../config/CharacterConfig.h"

enum class BoostType {
    STRENGTH,
    AGILITY
};

struct TemporaryBoost {
    BoostType type;
    uint8_t value;           // Cuánto suma (ej: +5)
    uint32_t timeLeftMs;     // Tiempo restante en milisegundos
};

class StatsComponent {
private:
    const FormulaEngine& formulaEngine;
    RaceConfig raceConfig;
    CharacterClassConfig classConfig;

    // Atributos principales
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
    
    // Buffs activos del Player
    std::vector<TemporaryBoost> activeBoosts; 
    // Método privado auxiliar para recalcular los techos de vida y maná
    void recalculateMaxStats();

public:
    // Constructor completo basado en tu nueva lista de atributos
    StatsComponent(const RaceConfig& race, const CharacterClassConfig& characterClass,
                   const PlayerConfig& playerBase,
                   const FormulaEngine& engine = FormulaEngine::getInstance());

    // --- GETTERS ---
    uint8_t getStrength() const {
        uint16_t total = strength;
        for (const auto& boost : activeBoosts) {
            if (boost.type == BoostType::STRENGTH) total += boost.value;
        }
        return static_cast<uint8_t>(total);
    }

    uint8_t getAgility() const {
        uint16_t total = agility;
        for (const auto& boost : activeBoosts) {
            if (boost.type == BoostType::AGILITY) total += boost.value;
        }
        return static_cast<uint8_t>(total);
    }

    uint8_t getIntelligence() const { return intelligence; }
    uint8_t getConstitution() const { return constitution; }

    uint16_t getHp() const { return health; }
    uint16_t getMaxHp() const { return max_health; }
    uint16_t getMana() const { return mana; }
    uint16_t getMaxMana() const { return max_mana; }
    uint32_t getExp() const { return exp; }
    uint16_t getLevel() const { return level; }

    // -- Modificadores de atributos --
    void addExperience(uint32_t amount);
    void takeDamage(uint16_t amount);
    void heal(uint16_t amount);
    void setHp(uint16_t newHp) { health = newHp;}
    void setMana(uint16_t newMana) { mana = newMana;}
    void restoreHp();
    bool consumeMana(uint16_t amount);
    void recoverMana(uint16_t amount);
    void restoreMana();

    // -- Manejo de Boosts --
    void addBoost(BoostType type, uint8_t value, uint32_t durationMs) {
        for (auto& boost : activeBoosts) {
            if (boost.type == type) {
                boost.timeLeftMs = std::max(boost.timeLeftMs, durationMs);
                boost.value = std::max(boost.value, value); // Mantiene el elixir más fuerte
                return;
            }
        }
        activeBoosts.push_back({type, value, durationMs});
    }

    void clearBoosts() {
        activeBoosts.clear();
    }

    void updateTicks(uint32_t dtMs) {
        for (auto it = activeBoosts.begin(); it != activeBoosts.end(); ) {
            if (it->timeLeftMs <= dtMs) {
                it = activeBoosts.erase(it); // Expira el boost
            } else {
                it->timeLeftMs -= dtMs;
                ++it;
            }
        }
    }
};
