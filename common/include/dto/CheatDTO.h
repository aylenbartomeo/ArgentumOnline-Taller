#ifndef CHEAT_DTO_H
#define CHEAT_DTO_H

#include <cstdint>

enum class CheatType : uint8_t {
    LEVEL_UP,
    DIE,
    GIVE_RANGED_WEAPONS,
    INFINITE_MANA,
    GIVE_GOLD,
    GIVE_ARMORS
};

struct CheatDTO {
    CheatType type;
};

#endif
