#ifndef CHEAT_DTO_H
#define CHEAT_DTO_H

#include <cstdint>

enum class CheatType : uint8_t { LEVEL_UP = 1, DIE = 2, GIVE_GOLD = 3 };

struct CheatDTO {
    CheatType type;
};

#endif
