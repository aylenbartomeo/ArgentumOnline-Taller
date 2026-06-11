#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include <SDL_mixer.h>

#include "common/include/dto/Snapshot.h"
#include "common/utils/types.h"

class AudioSystem {
public:
    static constexpr int MONSTER_SOUND_MAX_DIST = 10;
    static constexpr int MONSTER_SOUND_MIN_DIST = 3;  // distancia a la que suena al máximo
    static constexpr int MONSTER_SOUND_MAX_VOL = MIX_MAX_VOLUME;  // 128
    static constexpr int MONSTER_SOUND_MIN_VOL = 12;  // volumen mínimo audible (no cero)

    AudioSystem();
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;
    AudioSystem(AudioSystem&&) = delete;
    AudioSystem& operator=(AudioSystem&&) = delete;

    void toggleMute();

    void updateMonsterSounds(const SnapshotDTO& snapshot, uint32_t nowMs, uint32_t myId);

    void playResurrectSound();
    void playSwordAttackSound();
    void playMagicAttackSound();
    void playPickGoldSound();
    void playProjectileHitSound();

private:
    Mix_Music* bgMusic = nullptr;
    Mix_Chunk* resurrectSound = nullptr;
    Mix_Chunk* swordAttackSound = nullptr;
    Mix_Chunk* magicAttackSound = nullptr;
    Mix_Chunk* pickGoldSound = nullptr;
    Mix_Chunk* projHitSound = nullptr;

    std::unordered_map<NPCType, Mix_Chunk*> monsterSounds;
    std::unordered_map<uint32_t, uint32_t> nextSoundTime;

    bool isMuted = false;
    int lastVolume = MIX_MAX_VOLUME / 2;
};

#endif
