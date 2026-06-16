#ifndef FORMULA_ENGINE_H_
#define FORMULA_ENGINE_H_

#include <cstdint>

#include "RandomNumberGenerator.h"

class FormulaEngine {
private:
    FormulaEngine() = default;
    mutable RandomNumberGenerator rng;

public:
    ~FormulaEngine() = default;

    // Evitamos copias para garantizar que sea una instancia única
    FormulaEngine(const FormulaEngine&) = delete;
    FormulaEngine& operator=(const FormulaEngine&) = delete;

    // Método estático que devuelve la única instancia
    static FormulaEngine& getInstance() {
        static FormulaEngine instance;
        return instance;
    }

    // Calcula la vida máxima: Constitucion * FClaseVida * FRazaVida * Nivel
    uint16_t calculateMaxLife(uint16_t constitution, float class_factor, float race_factor,
                                uint16_t level) const;

    // Calcula el maná máximo: Inteligencia * FClaseMana * FRazaMana * Nivel
    uint16_t calculateMaxMana(uint16_t intelligence, float class_factor, float race_factor,
                                uint16_t level) const;

    // Calcula el daño bruto de un arma: Fuerza * rand(Min, Max)
    uint16_t calculateBaseDamage(uint16_t strength, uint16_t weapon_min,
                                   uint16_t weapon_max) const;

    // Calcula mitigación: rand(ArmaMin, ArmaMax) + rand(EscMin, EscMax) + rand(CascMin, CascMax)
    uint16_t calculateDefense(uint16_t armor_min, uint16_t armor_max, uint16_t shield_min,
                               uint16_t shield_max, uint16_t helmet_min, uint16_t helmet_max) const;

    // Evalúa si un ataque se esquiva: rand(0,1)^Agilidad < 0.001
    bool isAttackEluded(uint16_t agility) const;

    // Límite de oro seguro: 100 * Nivel^1.1
    uint32_t calculateSafeGoldLimit(uint16_t level) const;

    // Límite de XP para subir: 1000 * Nivel^1.8
    uint32_t calculateLevelUpLimit(uint16_t level) const;

    // XP por ataque exitoso: Danio * max(NivelDelOtro - Nivel + 10, 0)
    uint32_t calculateAttackXpGain(uint16_t damage, uint16_t attacker_level,
                                      uint16_t victim_level) const;

    // Calcula si es newbie (nivel <= 12)
    bool isNewbie(uint16_t level) const;

    // Valida si el nivel de PvP es permitido (diferencia <= 10)
    bool isPvpLevelValid(uint16_t attacker_level, uint16_t victim_level) const;

    // Calcula la pérdida de experiencia al morir: 25% del límite de nivel actual
    uint32_t calculateDeathExpLoss(int currentLevel) const;

    // Calcula el oro que se pierde al morir: max(0, OroActual - OroSeguro)
    uint32_t calculateExcessGoldDropped(uint32_t current_gold, uint32_t safe_limit) const;

    // Calcula el oro que suelta un NPC al morir: rand(0, 0.2) * VidaMaxNPC
    uint32_t calculateNpcGoldDrop(uint16_t npc_max_life) const;

    // Evalúa si un ataque es crítico: rand(0,1) < ProbabilidadCritico
    bool isCriticalAttack(float critical_probability) const;

    // Calcula recuperación pasiva: FRecuperacionRaza * SegundosTranscurridos
    float calculatePassiveRecovery(float race_recovery_factor, float seconds_elapsed) const;

    // Calcula recuperación por meditación: FMeditacionClase * Inteligencia * SegundosTranscurridos
    float calculateMeditationRecovery(float class_meditation_factor, uint16_t intelligence,
                                        float seconds_elapsed) const;

    // Calcula XP ganada por matar a un NPC: VidaMaxVictima * max(0, 10 - DiferenciaNiveles)
    uint32_t calculateKillXpGain(uint16_t victim_max_life, uint16_t attacker_level,
                                    uint16_t victim_level) const;
};

#endif  // FORMULA_ENGINE_H_
