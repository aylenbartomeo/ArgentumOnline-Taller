#include "RegenerationComponent.h"

RegenerationComponent::RegenerationComponent(StatsComponent& stats, StateComponent& state,
                                             const RaceConfig& race,
                                             const CharacterClassConfig& cls,
                                             const FormulaEngine& engine):
        stats(stats),
        state(state),
        raceRecoveryFactor(race.recoveryFactor),
        classMeditationFactor(cls.meditationFactor),
        canUseMagic(cls.canUseMagic),
        formulaEngine(engine) {}

void RegenerationComponent::tick(float secondsElapsed) {
    if (state.isGhost() || secondsElapsed <= 0.0f)
        return;

    // Recuperacion pasiva de vida: Vida = FRazaRecuperacion * segundos
    accumulatedHpRecovery +=
            formulaEngine.calculatePassiveRecovery(raceRecoveryFactor, secondsElapsed);
    if (accumulatedHpRecovery >= 1.0f) {
        const uint16_t hpRecovered = static_cast<uint16_t>(accumulatedHpRecovery);
        stats.heal(hpRecovered);
        accumulatedHpRecovery -= static_cast<float>(hpRecovered);
    }

    // Recuperacion de mana: Mana = FClaseMeditacion * Inteligencia * segundos
    if (!canUseMagic)
        return;
    if (state.isMeditating()) {
        accumulatedManaRecovery += formulaEngine.calculateMeditationRecovery(
                classMeditationFactor, stats.getIntelligence(), secondsElapsed);

        if (accumulatedManaRecovery >= 1.0f) {
            const uint16_t manaRecovered = static_cast<uint16_t>(accumulatedManaRecovery);
            stats.recoverMana(manaRecovered);
            accumulatedManaRecovery -= static_cast<float>(manaRecovered);
        }

        // El jugador sale de meditación automáticamente cuando llega al tope.
        if (stats.getMana() >= stats.getMaxMana()) {
            state.stopMeditating();
        }

    } else {
        // Fuera de meditación: recuperación pasiva de maná. Mana = FRazaRecuperacion * segundos
        accumulatedManaRecovery +=
                formulaEngine.calculatePassiveRecovery(raceRecoveryFactor, secondsElapsed);

        if (accumulatedManaRecovery >= 1.0f) {
            const uint16_t manaRecovered = static_cast<uint16_t>(accumulatedManaRecovery);
            stats.recoverMana(manaRecovered);
            accumulatedManaRecovery -= static_cast<float>(manaRecovered);
        }
    }
}
