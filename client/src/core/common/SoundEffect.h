#ifndef SOUND_EFFECT_H
#define SOUND_EFFECT_H

#include <cstdint>

enum class SoundEffect : uint8_t {
    RESURRECT,
    SWORD_ATTACK,
    MAGIC_ATTACK,
    PICK_GOLD,
    PROJ_HIT,
    PICK_ITEM,
    DROP_ITEM,
    EQUIP_WEAPON,
    LEVEL_UP,
    DIE,
    FLAUTE,
    BOW_SHOOT,
    DRINK_POTION,
};

#endif
