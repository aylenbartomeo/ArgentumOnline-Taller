#ifndef STATE_AUDIO_TRIGGER_H
#define STATE_AUDIO_TRIGGER_H

#include "common/include/dto/PlayerStatsDTO.h"

#include "AudioSystem.h"

struct StateChanges {
    bool tookDamage = false;
    bool gotHealed = false;
};

class StateAudioTrigger {
public:
    // Compara el estado anterior con el nuevo y dispara sonidos.
    StateChanges checkAndTrigger(const PlayerStatsDTO& oldStats, const PlayerStatsDTO& newStats,
                                 AudioSystem& audio);
};

#endif
