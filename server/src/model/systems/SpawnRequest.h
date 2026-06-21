#ifndef SPAWN_REQUEST_H
#define SPAWN_REQUEST_H

#include "../../common/utils/position.h"
#include "../../common/utils/types.h"
#include "../../config/MonsterConfig.h"

struct SpawnRequest {
    NPCType type;
    Position pos;
    const MonsterConfig* config;
};

#endif // SPAWN_REQUEST_H
