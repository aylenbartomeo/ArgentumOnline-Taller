#include "FormulaEngine.h"

#include <algorithm>
#include <cmath>

// ========================================================================
// ESTADÍSTICAS DE PERSONAJE
// ========================================================================

uint16_t FormulaEngine::calculateMaxLife(uint16_t constitution, float class_factor,
                                           float race_factor, uint16_t level) const {
    float result = static_cast<float>(constitution) * class_factor * race_factor *
                   static_cast<float>(level);
    return static_cast<uint16_t>(result);
}

uint16_t FormulaEngine::calculateMaxMana(uint16_t intelligence, float class_factor,
                                           float race_factor, uint16_t level) const {
    float result = static_cast<float>(intelligence) * class_factor * race_factor *
                   static_cast<float>(level);
    return static_cast<uint16_t>(result);
}

// ========================================================================
// EXPERIENCIA Y NIVELES
// ========================================================================

uint32_t FormulaEngine::calculateLevelUpLimit(uint16_t current_level) const {
    float result = 1000.0f * std::pow(static_cast<float>(current_level), 1.8f);
    return static_cast<uint32_t>(result);
}

uint32_t FormulaEngine::calculateAttackXpGain(uint16_t damage, uint16_t attacker_level,
                                                 uint16_t victim_level) const {
    int level_diff = static_cast<int>(victim_level) - static_cast<int>(attacker_level) + 10;
    uint32_t multiplier = static_cast<uint32_t>(std::max(level_diff, 0));
    return static_cast<uint32_t>(damage) * multiplier;
}

uint32_t FormulaEngine::calculateKillXpGain(uint16_t victim_max_life, uint16_t attacker_level,
                                               uint16_t victim_level) const {
    float random_factor = rng(0.0f, 0.1f);

    int level_diff = static_cast<int>(victim_level) - static_cast<int>(attacker_level) + 10;
    float multiplier = static_cast<float>(std::max(level_diff, 0));
    float result = random_factor * static_cast<float>(victim_max_life) * multiplier;
    return static_cast<uint32_t>(result);
}

bool FormulaEngine::isNewbie(uint16_t level) const { return level <= 12; }

bool FormulaEngine::isPvpLevelValid(uint16_t attacker_level, uint16_t victim_level) const {
    int diff = static_cast<int>(attacker_level) - static_cast<int>(victim_level);
    return std::abs(diff) <= 10;
}

uint32_t FormulaEngine::calculateDeathExpLoss(int currentLevel) const {
    uint32_t nextLevelLimit = calculateLevelUpLimit(currentLevel);
    return static_cast<uint32_t>(nextLevelLimit *
                                 0.25f);  // Pierde el 25% del límite del siguiente nivel
}

// ========================================================================
// ECONOMÍA Y DROPS (ORO)
// ========================================================================

uint32_t FormulaEngine::calculateSafeGoldLimit(uint16_t level) const {
    float result = 100.0f * std::pow(static_cast<float>(level), 1.1f);
    return static_cast<uint32_t>(result);
}

uint32_t FormulaEngine::calculateExcessGoldDropped(uint32_t current_gold,
                                                      uint32_t safe_limit) const {
    if (current_gold <= safe_limit) {
        return 0;
    }
    // El muerto conserva el límite seguro y deja caer el exceso
    return current_gold - safe_limit;
}

uint32_t FormulaEngine::calculateNpcGoldDrop(uint16_t npc_max_life) const {
    float drop_factor = rng(0.0f, 0.2f);
    return static_cast<uint32_t>(drop_factor * static_cast<float>(npc_max_life));
}

// ========================================================================
// COMBATE Y DAÑO
// ========================================================================

uint16_t FormulaEngine::calculateBaseDamage(uint16_t strength, uint16_t weapon_min_damage,
                                              uint16_t weapon_max_damage) const {
    int weapon_roll = rng(static_cast<int>(weapon_min_damage), static_cast<int>(weapon_max_damage));
    return strength * static_cast<uint16_t>(weapon_roll);
}

uint16_t FormulaEngine::calculateDefense(uint16_t armor_min, uint16_t armor_max,
                                          uint16_t shield_min, uint16_t shield_max,
                                          uint16_t helmet_min, uint16_t helmet_max) const {
    // Si un ítem no está equipado, sus mínimos y máximos inyectados serán 0, lo cual devuelve 0.
    int armor_roll = rng(static_cast<int>(armor_min), static_cast<int>(armor_max));
    int shield_roll = rng(static_cast<int>(shield_min), static_cast<int>(shield_max));
    int helmet_roll = rng(static_cast<int>(helmet_min), static_cast<int>(helmet_max));
    return static_cast<uint16_t>(armor_roll + shield_roll + helmet_roll);
}

bool FormulaEngine::isAttackEluded(uint16_t agility) const {
    float base_prob = rng(0.0f, 1.0f);
    float elude_roll = std::pow(base_prob, static_cast<float>(agility));
    return elude_roll < 0.001f;
}

bool FormulaEngine::isCriticalAttack(float critical_probability) const {
    return rng(0.0f, 1.0f) < critical_probability;
}

// ========================================================================
// REGENERACIÓN
// ========================================================================

float FormulaEngine::calculatePassiveRecovery(float race_recovery_factor,
                                                float seconds_elapsed) const {
    return race_recovery_factor * seconds_elapsed;
}

float FormulaEngine::calculateMeditationRecovery(float class_meditation_factor,
                                                   uint16_t intelligence,
                                                   float seconds_elapsed) const {
    return class_meditation_factor * static_cast<float>(intelligence) * seconds_elapsed;
}
