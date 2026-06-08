#ifndef CHEAT_DTO_H
#define CHEAT_DTO_H

#include <cstdint>

enum class CheatType : uint8_t {
    LEVEL_UP,
    DIE,
    GIVE_BOW,
    INFINITE_MANA,
    GIVE_GOLD,
};

struct CheatDTO {
    CheatType type;
};

#endif
