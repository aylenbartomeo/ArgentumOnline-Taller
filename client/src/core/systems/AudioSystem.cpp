#include "AudioSystem.h"

#include <algorithm>
#include <iostream>

namespace {
constexpr int AUDIO_FREQUENCY = 44100;
constexpr int AUDIO_CHANNELS = 2;
constexpr int AUDIO_CHUNK_SIZE = 2048;
constexpr int MUSIC_VOLUME = 64;
constexpr uint32_t MONSTER_SOUND_INTERVAL_MIN_MS = 8000;
constexpr uint32_t MONSTER_SOUND_INTERVAL_MAX_MS = 18000;
constexpr const char* MUSIC_PATH = "resources/audio/music/game_theme.mp3";
constexpr uint32_t MONSTER_SOUND_DURATION_MIN_MS = 5000;
constexpr uint32_t MONSTER_SOUND_DURATION_MAX_MS = 10000;

}  // namespace

AudioSystem::AudioSystem() {
    if (Mix_OpenAudio(AUDIO_FREQUENCY, MIX_DEFAULT_FORMAT, AUDIO_CHANNELS, AUDIO_CHUNK_SIZE) < 0) {
        std::cerr << "[AUDIO] Mix_OpenAudio error: " << Mix_GetError() << std::endl;
        return;
    }
    bgMusic = Mix_LoadMUS(MUSIC_PATH);
    if (!bgMusic) {
        std::cerr << "[AUDIO] No se pudo cargar la música: " << Mix_GetError() << std::endl;
        return;
    }
    Mix_PlayMusic(bgMusic, -1);
    Mix_VolumeMusic(MUSIC_VOLUME);
    Mix_AllocateChannels(16);

    const std::unordered_map<NPCType, std::string> soundPaths = {
            {NPCType::ZOMBIE, "resources/audio/monsters/zombie.ogg"},
            {NPCType::SKELETON, "resources/audio/monsters/skeleton.ogg"},
            {NPCType::GOLEM, "resources/audio/monsters/golem.ogg"},
            {NPCType::SPIDER, "resources/audio/monsters/spider.ogg"},
            {NPCType::GOBLIN, "resources/audio/monsters/goblin.ogg"},
            {NPCType::ORC, "resources/audio/monsters/goblin.ogg"},
    };

    resurrectSound = Mix_LoadWAV("resources/audio/player/resucitar.ogg");
    if (!resurrectSound) {
        std::cerr << "[AUDIO] No se pudo cargar sonido de resurreccion: " << Mix_GetError()
                  << std::endl;
    }

    for (const auto& [type, path]: soundPaths) {
        Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
        if (!chunk)
            std::cerr << "[AUDIO] No se pudo cargar sonido: " << path << " - " << Mix_GetError()
                      << std::endl;
        else
            monsterSounds[type] = chunk;
    }
}

AudioSystem::~AudioSystem() {
    if (bgMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgMusic);
    }
    if (resurrectSound) {
        Mix_FreeChunk(resurrectSound);
    }
    for (auto& [type, chunk]: monsterSounds) {
        if (chunk)
            Mix_FreeChunk(chunk);
    }
    monsterSounds.clear();
    Mix_CloseAudio();
    Mix_Quit();
}

void AudioSystem::toggleMute() {
    isMuted = !isMuted;
    if (isMuted) {
        lastVolume = Mix_VolumeMusic(-1);
        Mix_VolumeMusic(0);
    } else {
        Mix_VolumeMusic(lastVolume);
    }
}

static int volumeForDistance(float dist) {
    if (dist <= AudioSystem::MONSTER_SOUND_MIN_DIST)
        return AudioSystem::MONSTER_SOUND_MAX_VOL;
    if (dist >= AudioSystem::MONSTER_SOUND_MAX_DIST)
        return 0;

    const float t = (dist - AudioSystem::MONSTER_SOUND_MIN_DIST) /
                    static_cast<float>(AudioSystem::MONSTER_SOUND_MAX_DIST -
                                       AudioSystem::MONSTER_SOUND_MIN_DIST);

    const float falloff = (1.0f - t) * (1.0f - t);

    return static_cast<int>(AudioSystem::MONSTER_SOUND_MAX_VOL * falloff);
}

void AudioSystem::playResurrectSound() {
    if (!isMuted && resurrectSound) {
        const int chan = Mix_PlayChannel(-1, resurrectSound, 0);
        if (chan >= 0) {
            Mix_SetPanning(chan, 255, 255);
            Mix_Volume(chan, 60);
        }
    }
}

void AudioSystem::updateMonsterSounds(const SnapshotDTO& snapshot, uint32_t nowMs, uint32_t myId) {
    const auto playerIt = std::find_if(snapshot.players.begin(), snapshot.players.end(),
                                       [myId](const EntityDTO& p) { return p.id == myId; });

    if (playerIt == snapshot.players.end())
        return;

    const EntityDTO* me = &(*playerIt);

    for (const EntityDTO& monster: snapshot.monsters) {
        const NPCType npcType = static_cast<NPCType>(monster.entityTypeId);

        auto soundIt = monsterSounds.find(npcType);
        if (soundIt == monsterSounds.end())
            continue;

        // Distancia euclidiana
        const float dx = static_cast<float>(monster.x) - static_cast<float>(me->x);
        const float dy = static_cast<float>(monster.y) - static_cast<float>(me->y);
        const float dist = std::sqrt(dx * dx + dy * dy);

        const int vol = volumeForDistance(dist);
        auto& nextTime = nextSoundTime[monster.id];

        // Inicializar el temporizador si es un monstruo nuevo en pantalla
        if (nextTime == 0) {
            nextTime = nowMs + MONSTER_SOUND_INTERVAL_MIN_MS +
                       (rand() % (MONSTER_SOUND_INTERVAL_MAX_MS - MONSTER_SOUND_INTERVAL_MIN_MS));
            continue;
        }

        // Si es momento de reproducir el sonido
        if (nowMs >= nextTime) {
            if (vol > 0) {
                const int chan = Mix_PlayChannel(-1, soundIt->second, 0);

                if (chan >= 0) {
                    int leftVol = vol;
                    int rightVol = vol;

                    if (dist > 0) {
                        const float balance = dx / dist;
                        leftVol = static_cast<int>(vol * (1.0f - balance) / 2.0f + vol / 2.0f);
                        rightVol = static_cast<int>(vol * (1.0f + balance) / 2.0f + vol / 2.0f);
                    }
                    Mix_SetPanning(chan, leftVol, rightVol);
                    Mix_Volume(chan, vol);
                }
            }

            // Reprogramar el siguiente sonido para este monstruo
            nextTime = nowMs + MONSTER_SOUND_INTERVAL_MIN_MS +
                       (rand() % (MONSTER_SOUND_INTERVAL_MAX_MS - MONSTER_SOUND_INTERVAL_MIN_MS));
        }
    }

    // Limpiar temporizadores de monstruos muertos
    for (auto cleanupIt = nextSoundTime.begin(); cleanupIt != nextSoundTime.end();) {
        const bool alive = std::any_of(
                snapshot.monsters.begin(), snapshot.monsters.end(),
                [&](const EntityDTO& monster) { return monster.id == cleanupIt->first; });

        // Si el monstruo murió, simplemente lo quitamos del mapa de temporizadores
        cleanupIt = alive ? std::next(cleanupIt) : nextSoundTime.erase(cleanupIt);
    }
}
