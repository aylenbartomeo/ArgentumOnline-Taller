#ifndef CHARACTER_SPRITES_H
#define CHARACTER_SPRITES_H

#include "common/include/dto/Snapshot.h"

struct EntitySprite {
    const char* bodySheet;
    bool drawHead;
    int headOverlap;
};

EntitySprite spriteForEntity(EntityType type);

#endif
