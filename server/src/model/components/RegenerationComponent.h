#ifndef REGENERATION_COMPONENT_H
#define REGENERATION_COMPONENT_H

#include <cstdint>

#include "../../include/FormulaEngine.h"
#include "../config/CharacterConfig.h"

#include "StateComponent.h"
#include "StatsComponent.h"

class RegenerationComponent {
private:
    StatsComponent& stats;
    StateComponent& state;

    const float raceRecoveryFactor;
    const float classMeditationFactor;
    const bool canUseMagic;

    const FormulaEngine& formulaEngine;

public:
    RegenerationComponent(StatsComponent& stats, StateComponent& state, const RaceConfig& race,
                          const CharacterClassConfig& cls,
                          const FormulaEngine& engine = FormulaEngine::getInstance());

    // Llamado cada tick del game loop del servidor con los segundos transcurridos.
    // Aplica recuperación pasiva de vida, pasiva de maná y por meditación, respetando el estado de
    // player.
    void tick(float secondsElapsed);
};

#endif
