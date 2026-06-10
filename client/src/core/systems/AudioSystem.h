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

private:
    Mix_Music* bgMusic_ = nullptr;
};

#endif
