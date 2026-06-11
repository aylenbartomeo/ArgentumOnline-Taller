#ifndef STATE_AUDIO_TRIGGER_H
#define STATE_AUDIO_TRIGGER_H

#include "common/include/dto/PlayerStatsDTO.h"

#include "AudioSystem.h"

class StateAudioTrigger {
public:
    // Compara el estado anterior con el nuevo y dispara sonidos.
    // Retorna true si el jugador recibió daño
    bool checkAndTrigger(const PlayerStatsDTO& oldStats, const PlayerStatsDTO& newStats,
                         AudioSystem& audio);
};

#endif
