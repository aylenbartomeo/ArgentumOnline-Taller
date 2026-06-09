#ifndef CHARACTER_SPRITES_H
#define CHARACTER_SPRITES_H

#include <cstdint>

#include "common/include/dto/Snapshot.h"

struct EntitySprite {
    const char* bodySheet;
    bool drawHead;
    int headOverlap;
    const char* headSheet;
    int headSrcX;
    int headSrcY;
    int headSrcW;
    int headSrcH;
    int bodySrcX;
    int bodySrcY;
    int bodySrcW;
    int bodySrcH;
    int bodyScale = 100;
};

EntitySprite spriteForEntity(EntityType type, uint16_t spriteId);

#endif
