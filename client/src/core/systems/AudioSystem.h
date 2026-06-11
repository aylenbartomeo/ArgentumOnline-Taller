#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <SDL_mixer.h>

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;
    AudioSystem(AudioSystem&&) = delete;
    AudioSystem& operator=(AudioSystem&&) = delete;

    void toggleMute();

private:
    Mix_Music* bgMusic_ = nullptr;

    bool isMuted = false;
    int lastVolume = MIX_MAX_VOLUME / 2;
};

#endif
