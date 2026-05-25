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
    const uint16_t hpRecovered =
            formulaEngine.calculate_passive_recovery(raceRecoveryFactor, secondsElapsed);

    if (hpRecovered > 0) {
        stats.heal(hpRecovered);
    }

    // Recuperacion de mana: Mana = FClaseMeditacion * Inteligencia * segundos
    if (!canUseMagic)
        return;
    if (state.isMeditating()) {
        const uint16_t manaRecovered = formulaEngine.calculate_meditation_recovery(
                classMeditationFactor, stats.getIntelligence(), secondsElapsed);

        if (manaRecovered > 0) {
            stats.recoverMana(manaRecovered);
        }

        // El jugador sale de meditación automáticamente cuando llega al tope.
        if (stats.getMana() >= stats.getMaxMana()) {
            state.stopMeditating();
        }

    } else {
        // Fuera de meditación: recuperación pasiva de maná. Mana = FRazaRecuperacion * segundos
        const uint16_t manaRecovered =
                formulaEngine.calculate_passive_recovery(raceRecoveryFactor, secondsElapsed);

        if (manaRecovered > 0) {
            stats.recoverMana(manaRecovered);
        }
    }
}
