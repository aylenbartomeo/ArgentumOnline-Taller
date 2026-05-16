#ifndef STARTMOVEDTO_H
#define STARTMOVEDTO_H

#include <cstdint>

enum class Direction : uint8_t { UP, DOWN, LEFT, RIGHT };

struct StartMoveDTO {
    uint8_t direction = 0;

    StartMoveDTO() = default;

    explicit StartMoveDTO(Direction direction): direction(static_cast<uint8_t>(direction)) {}
};

#endif
