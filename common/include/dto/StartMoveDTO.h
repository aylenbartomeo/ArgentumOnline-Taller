#ifndef STARTMOVEDTO_H
#define STARTMOVEDTO_H

#include "CommandDTO.h"

struct StartMoveDTO {
    Movement direction = Movement::STOP;

    StartMoveDTO() = default;

    explicit StartMoveDTO(Movement direction): direction(direction) {}
};

#endif
