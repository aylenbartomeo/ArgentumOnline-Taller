#ifndef CHARACTER_SPRITES_H
#define CHARACTER_SPRITES_H

#include <cstdint>

#include "common/include/dto/Snapshot.h"

#include "CharacterAnimator.h"

struct EntitySprite {
    const char* bodySheet;
    bool drawHead;
    int headOverlap;
    const char* headSheet;
    int headSrcX;
    int headSrcY;
    int headSrcW;
    int headSrcH;
    int headStrideX = 0;
    int bodySrcX;
    int bodySrcY;
    int bodySrcW;
    int bodySrcH;
    int bodyScale = 100;
    bool customGrid = false;
    int bodyStrideX = 0;
    int bodyStrideY = 0;
    int bodyCols = 1;
    // 0 = estándar motor (DOWN=0, UP=1, LEFT=2, RIGHT=3)
    // 1 = boss estándar (DOWN=0, UP=1, LEFT=2, RIGHT=3)
    int rowOrder = 0;
};

EntitySprite spriteForEntity(EntityType type, uint8_t entityTypeId, uint32_t entityId = 0,
                             uint8_t stateId = 0);

FrameRect bodyFrameRectFor(const EntitySprite& sprite, Movement facing, int col);

#endif
