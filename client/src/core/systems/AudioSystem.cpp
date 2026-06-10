#include "AudioSystem.h"

#include <iostream>

namespace {
constexpr int AUDIO_FREQUENCY = 44100;
constexpr int AUDIO_CHANNELS = 2;
constexpr int AUDIO_CHUNK_SIZE = 2048;
constexpr int MUSIC_VOLUME = 64;
constexpr const char* MUSIC_PATH = "resources/audio/music/game_theme.mp3";
}  // namespace

AudioSystem::AudioSystem() {
    if (Mix_OpenAudio(AUDIO_FREQUENCY, MIX_DEFAULT_FORMAT, AUDIO_CHANNELS, AUDIO_CHUNK_SIZE) < 0) {
        std::cerr << "[AUDIO] Mix_OpenAudio error: " << Mix_GetError() << std::endl;
        return;
    }
    bgMusic_ = Mix_LoadMUS(MUSIC_PATH);
    if (!bgMusic_) {
        std::cerr << "[AUDIO] No se pudo cargar la música: " << Mix_GetError() << std::endl;
        return;
    }
    Mix_PlayMusic(bgMusic_, -1);
    Mix_VolumeMusic(MUSIC_VOLUME);
}

AudioSystem::~AudioSystem() {
    if (bgMusic_) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgMusic_);
    }
    Mix_CloseAudio();
    Mix_Quit();
}
