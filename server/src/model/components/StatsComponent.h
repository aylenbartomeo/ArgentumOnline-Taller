#pragma once
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

#include "../../common/utils/types.h"
#include "../../include/FormulaEngine.h"
#include "../config/CharacterConfig.h"

enum class BoostType { STRENGTH, AGILITY };

struct TemporaryBoost {
    BoostType type;
    uint8_t value;        // Cuánto suma (ej: +5)
    uint32_t timeLeftMs;  // Tiempo restante en milisegundos
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

    Race race;
    CharacterClass characterClass;
    // Método privado auxiliar para recalcular los techos de vida y maná
    void recalculateMaxStats();

public:
    // Constructor completo basado en tu nueva lista de atributos
    StatsComponent(Race raceEnum, CharacterClass classEnum, const RaceConfig& race,
                   const CharacterClassConfig& characterClass, const PlayerConfig& playerBase,
                   const FormulaEngine& engine = FormulaEngine::getInstance());

    // Constructor de test: mantiene compatibilidad sin enums
    StatsComponent(const RaceConfig& race, const CharacterClassConfig& characterClass,
                   const PlayerConfig& playerBase,
                   const FormulaEngine& engine = FormulaEngine::getInstance());

    // --- GETTERS ---
    uint8_t getStrength() const {
        uint16_t total = std::accumulate(
                activeBoosts.begin(), activeBoosts.end(), static_cast<uint16_t>(strength),
                [](uint16_t sum, const TemporaryBoost& boost) {
                    return sum + (boost.type == BoostType::STRENGTH ? boost.value : 0);
                });
        return static_cast<uint8_t>(total);
    }

    uint8_t getAgility() const {
        uint16_t total = std::accumulate(
                activeBoosts.begin(), activeBoosts.end(), static_cast<uint16_t>(agility),
                [](uint16_t sum, const TemporaryBoost& boost) {
                    return sum + (boost.type == BoostType::AGILITY ? boost.value : 0);
                });
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

    uint32_t getExpIntoCurrentLevel() const {
        uint32_t prev = level > 1 ? formulaEngine.calculate_level_up_limit(level - 1) : 0;
        return exp > prev ? exp - prev : 0;
    }
    uint32_t getExpForCurrentLevel() const {
        uint32_t prev = level > 1 ? formulaEngine.calculate_level_up_limit(level - 1) : 0;
        return formulaEngine.calculate_level_up_limit(level) - prev;
    }

    // -- Modificadores de atributos --
    void addExperience(uint32_t amount);
    void takeDamage(uint16_t amount);
    void heal(uint16_t amount);
    void setHp(uint16_t newHp) { health = newHp; }
    void setMana(uint16_t newMana) { mana = newMana; }
    void restoreHp();
    bool consumeMana(uint16_t amount);
    void recoverMana(uint16_t amount);

    Race getRace() const { return race; }
    CharacterClass getCharacterClass() const { return characterClass; }


    // --- Restauracion desde persistencia ---
    void restoreFromPersist(uint16_t savedHp, uint16_t savedMana, uint32_t savedExp,
                            uint16_t savedLevel) {
        level = savedLevel;
        exp = savedExp;
        recalculateMaxStats();  // Recalcula max_health y max_mana según el nivel restaurado
        health = std::min(savedHp, max_health);
        mana = std::min(savedMana, max_mana);
    }

    void restoreMana();

    // -- Manejo de Boosts --
    void addBoost(BoostType type, uint8_t value, uint32_t durationMs) {
        auto it = std::find_if(activeBoosts.begin(), activeBoosts.end(),
                               [type](const TemporaryBoost& boost) { return boost.type == type; });
        if (it != activeBoosts.end()) {
            it->timeLeftMs = std::max(it->timeLeftMs, durationMs);
            it->value = std::max(it->value, value);  // Mantiene el elixir más fuerte
            return;
        }
        activeBoosts.push_back({type, value, durationMs});
    }

    void clearBoosts() { activeBoosts.clear(); }

    void updateTicks(uint32_t dtMs) {
        for (auto it = activeBoosts.begin(); it != activeBoosts.end();) {
            if (it->timeLeftMs <= dtMs) {
                it = activeBoosts.erase(it);  // Expira el boost
            } else {
                it->timeLeftMs -= dtMs;
                ++it;
            }
        }
    }
};
