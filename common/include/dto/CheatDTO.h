#ifndef CHEAT_DTO_H
#define CHEAT_DTO_H

#include <cstdint>

enum class CheatType : uint8_t { LEVEL_UP = 1, DIE = 2 };

struct CheatDTO {
    CheatType type;
};

#endif
